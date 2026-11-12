# FrameGraph plan

Goal: replace the per-command "driver emulation" hazard tracking with a frame graph that describes the whole GPU frame as a DAG of tasks. With every resource usage known up front, barriers are computed once per compiled graph and only issued where they're actually needed.

## How hazard tracking works today

**Global state per resource.** Every `GpuResource` stores its last known state per subresource:
- DX12: `GpuResourceDataDX12::myHazardData` (`GpuResourceBarrierDataDX12`). Each subresource holds `myState` (`GpuTextureState`/`GpuBufferState` bits, mapped to enhanced-barrier sync/access/layout by `GetTexture/BufferBarrierParams()`) and `myContext` (the owning queue type or `SHARED_READ`).
- Vulkan: `GpuResourceHazardDataVk`, which stores image layout, access mask and context per subresource.

**Recording.** Every command that touches a resource asks for the state it needs:
- Commands: clears, copies, render-target and vertex/index binding, `UpdateTextureData`, and `PrepareResourceShaderAccess()` for bindless SRV/UAV/RTAS views.
- These go through `CommandListDX12::TrackTexture/BufferSubresourceTransition()` and `CommandListVk::TrackSubresourceTransition()`.
- Each command list keeps a private `myLocalHazardData` hash map (resource → per-subresource `myFirstDstState`, `myState`, `myWasUsed`, `myWasWritten`, `myIsSharedReadState`).
- After a resource's first use on a list, transitions are batched into `myPendingTexture/BufferBarriers` and flushed before the next draw, dispatch or copy.
- On a resource's *first* use on a list, its state at execution time is unknown. So only `myFirstDstState` is remembered and no barrier is recorded.

**Submission.** `CommandQueue*::ResolveResourceHazardData()` runs before `ExecuteCommandLists`:
- It compares each subresource's global state with the list's `myFirstDstState`.
- It records the missing "patch" barriers into an extra command list and submits that first.
- It then copies the list's final local states into the global state.

**Cross-queue.** Handled by hand: `TransitionResource(TO_SHARED_CONTEXT_READ)` plus the user calling `StallForQueue`/`StallForFence`.

**Costs and limitations**
- A hash-map lookup and per-subresource loops on every resource use.
- An extra command list and submit whenever patching is needed.
- No knowledge of future usage: barriers are conservative (`ALL_SHADING` sync scopes, global UAV barriers), batched only within one command list, and never split.
- Mutable global state on resources makes parallel recording fragile.
- No transient resource lifetimes, so no memory aliasing.

## Revised design (decided 2026-10-29)

This supersedes the Phase 0 prototype (explicit `Link()`, static-only tasks, `AddInput`/`AddOutput`, "fixed resources").

### Principles
- **Resource accesses fully define the graph.** No explicit linking.
- **Tasks are set up in execution order.** Each task's static `Setup()` returns an instance of its own class. The task stores the handles it got back as members, and the caller passes them on to the next task's `Setup()`.
- **Write-only versioning.** A resource is identified by a handle, and each `Write()` creates a new version, i.e. a new handle. `Read()` returns the same version.
- **Resources are virtual during setup.** Handles are empty shells. Views are created when the graph is compiled and handed to tasks only in `Execute()`.
- **Per-frame task data vs. compiled data.** The task instance is recreated every frame and holds everything the compilation doesn't care about (handles, constants, pipelines). Its `RenderData` holds what the compilation produces (views); it lives in the compiled graph and survives across frames. `Execute()` gets both. Overlapping the setup of frame N with the execution of frame N-1 would use two builders.
- **Compile on change.** `Setup()` runs every frame, which is cheap. `Compile()` (called by `Execute()`) first computes the hash of the declared structure and returns right away if it equals the hash of the compiled graph. Otherwise it rebuilds the barriers, views and render data.
- **First version without allocation.** All resources are external. Every frame, after `Reset()` and before the tasks are set up, the caller registers them with `RegisterExisting( handle )`, which returns the handle of version 0 (produced before the graph, contents defined). The backbuffer is registered with `RegisterBackBuffer()`. Later, tasks can create resources themselves (`Create( desc )`).
- **Naming:** larger classes use the `FrameGraph` prefix, small structs and helper types the `Fg` prefix (`FgResourceHandle`, `kFg...`). "Task" instead of "job".
- **Layering.** The FrameGraph is built on top of the command lists and `RenderCore`, which know nothing about it. It should stay easy to move it into its own project on top of `fancy_core`. Generic pieces it needs, such as the access enum and its helpers, live in the rendering backend.
- **Coexistence with the legacy tracker.** Hazard tracking becomes a property of each command list instead of a program-wide switch. The graph records on lists that have it turned off, and all other code keeps using automatic tracking.

### Task API

```cpp
class TonemapTask {
public:
  struct RenderData {  // Filled when the graph is compiled, by Compile()
    TextureView * myDest = nullptr;
    TextureView * mySource = nullptr;
  };

  static TonemapTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aDest, FgResourceHandle aSource, float anExposure ) {
    TonemapTask * task = nullptr;
    const uint taskIndex = FgAddTask< TonemapTask >( aBuilder, "Tonemap", CommandListType::Graphics, &task );
    task->myDest = aDest;
    task->mySource = aSource;
    aBuilder.Write( taskIndex, task->myDest, RESOURCE_ACCESS_RENDER_TARGET );
    aBuilder.Read( taskIndex, task->mySource, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
    task->myExposure = anExposure;
    return *task;
  }

  static void Compile( const TonemapTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
    someRenderData.myDest = aContext.GetRtv( aTask.myDest );
    someRenderData.mySource = aContext.GetSrv( aTask.mySource );
  }

  static void Execute( CommandList * aCommandList, const TonemapTask & aTask, const RenderData & someRenderData );

  FgResourceHandle myDest;
  FgResourceHandle mySource;
  float myExposure = 1.0f;
};
```

- **A task class has:**
  - the FrameGraph handles and all per-frame data (constants, pipelines, ...) as members, set in `Setup()` every frame;
  - a `RenderData` struct (trivially destructible) with what the graph compiles, filled by `Compile()` when the graph is compiled;
  - `static Execute( CommandList *, const T &, const RenderData & )`.
- **A task class must be trivially destructible.** Its instances are never destructed: `Reset()` just rewinds the task allocator.
- **`uint AddTask( const FgTaskCreateInfo &, void ** aTaskMemoryOut )`** (not templated) allocates the task instance in the builder's task allocator, returns the task's index and writes the uninitialized memory to `aTaskMemoryOut`. All functions that declare something for a task take this index; nothing implicitly applies to the task added last, so a series of tasks can be added first and their accesses declared afterwards. The create info holds:
  - the name and the queue type;
  - size and alignment of the task and of its `RenderData`;
  - the type-erased `Execute` and `Compile` functions, which take `void *` and cast back to the task's own types.
- **`FrameGraphTaskHelper.h`** (optional) generates all of this for task classes with the usual layout:
  - `FgMakeTaskCreateInfo< T >( name, queueType )` fills the create info, with `static_assert`s for trivially destructible task and `RenderData`;
  - `FgExecuteTask< T >` / `FgCompileTask< T >` are the casting functions;
  - `uint FgAddTask< T >( builder, name, queueType, T ** taskOut )` adds the task, placement-news a default-constructed `T` into its memory and returns the task index.
  The builder itself has no templates, so later generated tasks (see "Follow-ups") can fill the create info without the helper.
- **`FrameGraphCompileContext`** resolves the task's handles in `Compile()`: `GetRtv()`, `GetDsv()`, `GetSrv()`, `GetUav()` for textures, `GetBufferSrv()`, `GetBufferUav()` for buffers, and `GetResource()`. It asserts that the task declared the access and that the view type matches the access, and creates views on demand.
- Compute tasks record their queue type but run on the single graphics command list for now, with barriers using compute sync scopes. Async compute comes later.

### Resource API

Inside a task's `Setup()`, with the handles the caller passed in:

| Call | Meaning |
|---|---|
| `Read( taskIndex, handle, access )` | Records a read of the handle's range by that task. Asserts on a write access. |
| `Write( taskIndex, handle, access )` | Records a write of the handle's range by that task. Asserts on a read access. |

Handles are unversioned since 2026-11-08 (see "Next phase: subresources"); the description of versions below is historical.

Before the tasks are set up, the caller registers all external resources every frame: `RegisterExisting( TextureHandle )`, `RegisterExisting( GpuBufferHandle )` and `RegisterBackBuffer()`. The returned handles are passed on to the tasks' `Setup()` functions.

- The returned handle is version 0, produced before the graph, with defined contents. Registering the same resource twice asserts.
- The builder requires a `RenderOutput` (constructor parameter). Its current backbuffer has no engine handle (`RenderOutput` stores the swap chain textures directly) and is recognized by comparing the resource pointer with it. Its view comes from the `RenderOutput` every frame.
- For all other resources, `FrameGraphResourceBase` stores the engine handle's index and generation. Both go into the declaration hash, so a recreated resource (new generation, even at the same pool slot or address) recompiles the graph. The compiled graph keeps them per resource (`FrameGraphCompiledResource`) and asserts they still match when it hands out views.
- `ReleaseCompiledGraph()` has to be called before destroying a resource that the compiled graph has views of.

**`FgResourceHandle`:** a registered resource (`myBaseIndex`), a `SubresourceRange` and the builder's `myFrameGraphGenerationIndex` at creation. The generation index counts the builder's `Reset()` calls (i.e. frames), so a handle from a previous frame asserts.

**Resource data:** each version is one `FrameGraphResource` (producer task, consumer tasks, next version, version number). The data that exists once per resource, shared by all its versions, lives in `FrameGraphResourceBase` (name, resource pointer, latest version; later subresources). A version points to its base by index.

**`ResourceAccess` (in `RenderEnums.h`), per shader stage:**
- Render targets and depth: `RENDER_TARGET`, `DEPTH_WRITE`, `DEPTH_READ`.
- Shader reads: `VERTEX_SHADER_READ`, `FRAGMENT_SHADER_READ`, `COMPUTE_SHADER_READ`, `RAYTRACING_SHADER_READ`.
- Shader writes (UAV): `VERTEX_SHADER_WRITE`, `FRAGMENT_SHADER_WRITE`, `COMPUTE_SHADER_WRITE`, `RAYTRACING_SHADER_WRITE`.
- Copies: `COPY_SRC`, `COPY_DST`.
- Buffer-only: `VERTEX_BUFFER`, `INDEX_BUFFER`, `INDIRECT_ARGUMENT`, `RAYTRACING_ACCELERATION_STRUCTURE_READ`.
- Final state only: `PRESENT`.
- Each access maps to DX12 enhanced-barrier sync, access and layout, and to a Vulkan stage, access mask and image layout.
- Helpers are on `RenderCore`: `IsWriteAccess()`, `IsShaderReadAccess()`, `IsShaderWriteAccess()`, `GetResourceAccessViewType()` (the `GpuResourceViewType` an access needs) and `GetResourceAccessName()`.

### Building the edges
For each resource:
- **Read after write:** a consumer of version v depends on the producer of v.
- **Write after read:** the producer of v + 1 depends on all consumers of v.
- **Write after write:** the producer of v + 1 depends on the producer of v.

All edges point from earlier to later setup order, so the Kahn sort (earliest ready task first) currently always returns the setup order and can't find a cycle. It's a placeholder for a real scheduler that reorders independent tasks.

### Compiling (only when the structure hash changes)

`Compile()` is its own step after `Setup()` and `BuildFrameGraph()`. Its first action is `ComputeDeclarationHash()`; the hash is not built up during setup. If it matches the compiled one, `Compile()` returns early.

**Hash over:**
- each task's name, queue type and execute thunk;
- each access's base index, subresource range and access type;
- each registered resource's type and engine handle (index and generation). The backbuffer has no handle and always contributes the same key.

**The output is a `FrameGraphCompiledGraph` with:**
- **Task order.**
- **One `RenderData` per task,** allocated in the compiled graph's own linear allocator and filled by the task's `Compile()`. Tasks that access the backbuffer are compiled again every frame, because the swap chain image changes.
- **Views,** created on demand and shared between tasks, keyed by kind and subresource range. All views live in one builder-wide array (`myCompiledViews`, at most `kFgMaxNumResourceViewsPerFrame`), and the views of a resource form a linked list starting at `FrameGraphCompiledResource::myFirstViewIndex`, so finding a view only walks that resource's few views. A view is identified by its kind (SRV, UAV, RTV, DSV, read-only DSV); the format will be added once accesses can request one. Views are owned by the compiled graph and destroyed on recompile.
- **Member naming:** everything that belongs to the compiled graph is called `myCompiled...` and declared together in the builder.
- **Barrier lists per task.** Between two accesses inside the graph, the transition is known at compile time.
- **Boundary per resource:** the first access of each subresource (`myCompiledInitialAccesses`, handed over by a tracked list before the graph) and closing barriers back to it (`myCompiledFinalBarriers`), see "Next phase: subresources".

### Executing (every frame)
1. Begin a graphics command list with hazard tracking turned off, and resolve the external resources.
2. For each task in order: issue its barrier batch through a new explicit barrier entry point on `CommandList`, then call `Execute()` with the current frame's task instance and its compiled `RenderData`.
3. Issue the closing barriers (last access back to first access per subresource) and submit. Before step 1, a separate tracked list hands the resources over in the state of their first access.

Task instances come from the current frame's `Setup()`, `RenderData` from the compiled graph, both indexed by task index.

### Hazard tracking per command list

**Replaces the program-wide switch:**
- Remove `RenderPlatformProperties::myEnableAutomaticHazardTracking`, `RenderCore::IsAutomaticHazardTrackingEnabled()` and `-noHazardTracking`.
- Instead, `RenderCore::BeginCommandList( type, flags )` takes an optional `CommandListFlags::NO_HAZARD_TRACKING` (or a bool), stored on the `CommandList`.
- The early return in `Track*SubresourceTransition()` checks that member.
- `RenderCore::BeginCommandList()` sets it every time, so a pooled list never carries it over.
- On a list without hazard tracking, `TransitionResource()` and `PrepareResourceShaderAccess()` assert: such lists only get explicit barriers through `TransitionResourceForAccess()`.
- I'd rather not do a scoped on/off within one list. Resources touched inside the scope would leave the legacy tracker's local state stale for the rest of the list.

**Boundaries with legacy lists:** see "Compiling and executing" in "Next phase: subresources". The graph's list doesn't touch the hazard data or the submit-time patching; a tracked handover list before the graph and closing barriers at its end keep the legacy state correct.

### Task memory

There's no suitable existing allocator:
- `SmallObjectAllocator<T>` is a pool for one type only. It's unused and broken: `( uint8 ) newPage` should be `( uint8 * )`, and its entries aren't aligned. It gets removed, together with `AlignedStorage.h`, which only it uses and which doesn't actually align anything.
- `PagedLinearAllocator` hands out offset ranges for GPU heaps and descriptor indices, not CPU memory.

Plan:
- Add a small linear arena, `Common/LinearAllocator.h`. It has a fixed-capacity, aligned byte buffer, `Allocate( size, alignment )` and `Reset()`.
- The builder holds two of capacity `kFgTaskMemorySize` (task instances and compiled `RenderData`). The containers of the graph have an inline capacity (`kFgMaxNumTasks`, `kFgMaxNumResources`, ...) but may overflow to the heap.
- Tasks and their `RenderData` must be trivially destructible (checked by `FrameGraphTaskHelper.h`), so `Reset()` just rewinds the arena.
- `AddTask()` returns `kFgInvalidIndex` and writes `nullptr` to `aTaskMemoryOut` without adding the task when the arena is full (after asserting); `Read()` / `Write()` ignore invalid task indices.

### Test: four quadrants

**Resources** (created by the test, registered every frame; the texture is recreated by a button):
- `"fgQuadrants"`: a 512×512 RGBA8 texture, shader-writable and usable as a render target (not the backbuffer).
- `"fgQuadrantColors"`: a buffer with four `float4` colors, shader-writable.

**Tasks.** Every step changes the access of at least one resource:

| # | Task | Queue | Accesses |
|---|---|---|---|
| 1 | `StoreColorsTask` | compute (on gfx for now) | colors buffer: `COMPUTE_SHADER_WRITE` (raw UAV) |
| 2 | `FillQuadrantComputeTask` top left | compute | texture: `COMPUTE_SHADER_WRITE` (UAV); colors: `COMPUTE_SHADER_READ` (raw SRV) |
| 3 | `FillQuadrantRasterTask` top right | gfx | texture: `RENDER_TARGET` (viewport + scissor, fullscreen triangle); colors: `FRAGMENT_SHADER_READ` |
| 4 | `FillQuadrantComputeTask` bottom left | compute | texture: `COMPUTE_SHADER_WRITE`; colors: `COMPUTE_SHADER_READ` |
| 5 | `FillQuadrantRasterTask` bottom right | gfx | texture: `RENDER_TARGET`; colors: `FRAGMENT_SHADER_READ` |
| 6 | `BlitToBackBufferTask` | gfx | texture: `FRAGMENT_SHADER_READ`; backbuffer: `RENDER_TARGET` |

- The fill tasks get their color only from the buffer, by quadrant index. Wrong colors mean a broken buffer write or read.
- Tasks 2-5 form a write-after-write chain on the texture, and all of them read the same buffer version.
- Covered: buffer UAV and SRV (compute and fragment), texture UAV, render target (not the backbuffer), texture SRV, backbuffer.

**Shaders:** `resources/shaders/Tests/FrameGraph.hlsl`, bindless like `HazardTracking.hlsl`. Variants: `STORE_COLORS_CS`, `FILL_QUADRANT_CS`, `FILL_COLOR` and `BLIT` (vertex + fragment).

**ImGui:**
- A checkbox per quadrant task. Unchecking one removes the task, so the structure hash changes and the graph is recompiled. That quadrant then keeps its previous contents.
- A "Recreate texture" button creates a new texture, which changes the hash and recompiles.
- A display of the hash and compile count.

**Expected result:** four colored quadrants on screen, and a clean D3D12 debug layer and Vulkan validation in both runs.

**Further test ideas (later):**
- **Write after read:** add a task that reads the texture (copy to a second texture) between two fill tasks. The second fill task must then wait for the consumer.
- **Coexistence:** a legacy-tracked list writes the texture before the graph runs, for example an ImGui-driven clear. This checks the boundary patching at submit time.
- **Read-modify-write:** an invert pass as a single `Write( COMPUTE_SHADER_WRITE )` that reads and writes the same UAV.

## Implementation steps

1. **Cleanup and arena:**
   - delete `SmallObjectAllocator.h` and `AlignedStorage.h`, and add `LinearAllocator`;
   - make hazard tracking a per-command-list flag and remove the global switch and `-noHazardTracking`.
2. **Types:**
   - `FgResourceHandle` (resource index, generation index) and `ResourceAccess` with its helpers on `RenderCore`;
   - `FrameGraphResource` per version (producer task, consumer tasks, access per consumer and producer) and `FrameGraphResourceBase` per resource;
   - task records (name, queue type, instance pointer, thunks, access list);
   - rename job to task.
3. **Builder:**
   - remove `Link`, `AddInput`/`AddOutput`, `AddFixedResource`/`GetFixedResource`;
   - add `RegisterExisting` (per frame, returns the handle), `AddTask( FgTaskCreateInfo )`, `Read` and `Write`;
   - derive the edges from the versions and sort;
   - compute the declaration hash at the start of `Compile()`, and return early if it didn't change.
4. **Compiled graph:** `RenderData` per task, view creation, and barrier lists.
5. **Barriers on CommandList:**
   - an explicit barrier entry point for DX12 and Vulkan;
   - access mapping tables;
   - (superseded) boundary states written into the list's local hazard data; now a tracked handover list before the graph and closing barriers at its end.
6. **Tasks and shaders:**
   - `FillQuadrantComputeTask`, `FillQuadrantRasterTask`, `BlitToBackBufferTask` and `FrameGraph.hlsl` (see "Test: four quadrants");
   - remove `ClearRenderTargetTask`, `OpaquePassTask` and `TonemapTask`, or keep the latter two as empty examples with the new signatures.
7. **Test and viewer:**
   - `Test_FrameGraph` as described above;
   - the viewer shows the versions and accesses in its tooltip.
8. **Validate:** run under the D3D12 debug layer and the Vulkan validation layers.

## Status (2026-10-29)

Steps 1-8 are implemented and not committed.

**Validation:**
- `Test_FrameGraph` renders all four quadrants on DX12 (`-DebugLayer`) and Vulkan (`-vulkan -DebugLayer`).
- Neither layer reports a barrier or layout error. The graph compiles once and is reused every frame after that.
- One DX12 error remains, reported once per texture (EXECUTION ERROR #1422). A render-target texture needs a discard or clear before its first use (see "Next").
- **Task structure (2026-10-30, revised 2026-11-04):** `FrameGraphExecuteParams` was replaced by a per-task `RenderData` filled by the task's `Compile()`; per-frame data lives in the task itself (see "Task API").
- **Buffers (2026-10-31):** buffers get raw SRV/UAV views over the whole buffer. The extended test (6 tasks, 5 views) runs on DX12 and Vulkan without new debug layer or validation messages; only the known one-time DX12 initialization error remains.
- **Per-frame registration (2026-11-01):** resources are registered every frame and return the handle. `CreateFromExisting()`, `GetBackBuffer()` and the lookup by name are gone.
- **Review fixes (2026-11-06):** registration by engine handle (generation in the hash), `RegisterBackBuffer()`, required `RenderOutput`, Vulkan same-access write barrier, asserts on implicit transitions in lists without hazard tracking, plain Vulkan layouts, early returns on failed allocations and view creation.

**Files:**
- `fancy_core/Rendering/FrameGraph/`: `FrameGraphBuilder`, `FrameGraphTypes`, `FrameGraphTaskHelper.h` (optional templated task helpers)
- `ResourceAccess` in `RenderEnums.h`, its helpers in `RenderCore`
- `fancy_core/Common/LinearAllocator.h`
- `fancy_imgui/FrameGraphViewer`
- `Tests/Test_FrameGraph`
- `resources/shaders/Tests/FrameGraph.hlsl`

**Barrier entry point:** `CommandList::TransitionResourceForAccess( const GpuResource *, const SubresourceRange &, ResourceAccess src, ResourceAccess dst )`, in `CommandListDX12` and `CommandListVk`. Nothing in the backend refers to the FrameGraph, and no tracked state is read or written.

**Engine fixes found by the test:**
- **DX12 pipeline state:** with an unchanged pipeline, a dispatch after a draw on the same list ran with the graphics pipeline still bound. Graphics, compute and raytracing share one pipeline-state slot, so applying one of them now marks the others as dirty.
- **`CommandList::SetClipRect()`:** the comment said x, y, width, height, but the code takes left, top, right, bottom.

**Differences from the design above:**
- **Barrier precision:** accesses map to the existing `GpuTextureState`/`GpuBufferState` values and Vulkan access masks and layouts. Per-stage precision stops at fragment-shader reads versus all other shader reads. Sync scopes per stage come later.
- **Barrier decisions:** a compiled barrier (explicit source and destination) is issued whenever the access changes or it's a write. `TransitionResourceForAccess()` skips reads whose accesses map to the same backend state; writes always get a barrier (e.g. UAV → UAV).
- **Same-access writes:** with explicit source and destination, two writes in a row with the same access get a memory barrier with the same access and layout on both sides, on both backends.
- **Known gap in the legacy patching:** when a legacy list's last write equals the graph's first access (e.g. both UAV writes), the submit-time patching of the handover list sees no state change and adds no barrier, so the two writes aren't ordered. This is existing behaviour of the legacy tracking for all lists.
- **Vulkan layouts:** `TransitionResourceForAccess()` uses the plain optimal layout of each access (e.g. `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`, `TRANSFER_SRC_OPTIMAL`) instead of `ResolveImageLayout()`, which reads the global hazard state at record time. That function only differs for resources in the `SHARED_READ` state, which the graph doesn't support until the multi-queue phase. `GENERAL` everywhere was rejected: it would lose depth and color compression.
- **No `PRESENT` access:** ImGui draws after the graph, and the present path already transitions the backbuffer.
- **Buffers:** buffer views are raw (`ByteAddressBuffer`) over the whole buffer. Structured and typed buffer views come later.
- **Recompiling:** a recompile waits for the graphics queue to go idle before deleting the old views.

**Next:**
- Discard the contents of a resource at its first access when they're known to be undefined (this needs a way to declare that, see "Open questions"): on DX12, an enhanced barrier from `LAYOUT_UNDEFINED` with `D3D12_TEXTURE_BARRIER_FLAG_DISCARD`; on Vulkan, `oldLayout = UNDEFINED`. This would fix the remaining DX12 error: the test texture is never written before its first use as a render target.

## Next phase: subresources (revised 2026-11-08)

Goal: tasks access any range of subresources (mips, planes; array slices are carried by the primitives and work for RTVs and DSVs, but SRVs and UAVs of array textures don't work yet because `TextureView::GetGlobalResourceType()` has no array case and the builder always uses the resource's dimension). Most accesses still cover the whole resource.

### Decision: unversioned handles, program order is the only source of truth
- Versions (Frostbite style, `Write()` returns version + 1) only re-encoded what the setup order already says: within a resource, the version chain is the order of the writes. With subresources they would additionally need staleness checks per subresource (write stamps), a `GetLatest()` to merge per-mip versions back into one handle, and asserts on outdated handles.
- Instead, a handle names a registered resource plus a `SubresourceRange`, like a variable in code. The builder keeps, per subresource, the last writer task and the readers since that write, updated in setup order when `BuildFrameGraph()` replays the recorded accesses, and derives the edges from that (like a compiler deriving SSA form internally).
- Same model as the Activision task graph (REAC 2023): `handle.Mip( n )` narrows a handle, and downstream code keeps using the whole-resource handle after per-mip writes.
- Lost compared to versions: reading old contents after a newer write was declared (pointless with in-place writes to external resources), and detecting two tasks that both "fork" from the same version (now simply ordered by setup order).

### Handles
```cpp
struct FgResourceHandle {
  SubresourceRange mySubresources;
  uint myBaseIndex = kFgInvalidIndex;
  uint myFrameGraphGenerationIndex = 0u;

  bool IsValid() const;
  FgResourceHandle Mips( uint aFirstMip, uint aNumMips = 1u ) const;      // Narrows the range, asserts it lies inside this handle's range
  FgResourceHandle Planes( uint aFirstPlane, uint aNumPlanes = 1u ) const;
  FgResourceHandle ArraySlices( uint aFirstIndex, uint aNumIndices = 1u ) const;
};
```
- `RegisterExisting()` / `RegisterBackBuffer()` return a handle over all subresources. Buffers have one subresource.
- `Read( taskIndex, handle, access )` and `Write( taskIndex, handle, access )` record the access for that task and return nothing: returning a handle would suggest a new version. Tasks store the handle they were given. `FrameGraphResource` (versions) and `GetResources()` are removed.
- `SubresourceRange` gets `Overlaps()` and `Contains()`. Iteration uses the existing `SubresourceIterator`, indexing uses `GpuResource::GetSubresourceIndex()`.

### Edges, derived when building
- `Read()` / `Write()` only validate and record the access in the task. Setup stays a plain recording step.
- `BuildFrameGraph()` replays all recorded accesses in setup order with a per-subresource state `FgSubresourceState { myWriterTask, myReaderTasks }` (one flat builder array, `mySubresourceStates`; each `FrameGraphResourceBase` gets its `myFirstSubresourceState` there):
  - a read gets an edge from the subresource's writer (read after write) and is added to its readers;
  - a write gets edges from the writer (write after write) and all readers since (write after read), then becomes the writer and clears the readers.
- The replay follows the order of the tasks (`AddTask()`), not the order of the `Read()` / `Write()` calls, which may come in any order before `BuildFrameGraph()`.
- Then it sorts (Kahn).
- A task may access the same resource more than once if the ranges don't overlap (e.g. read mip N-1, write mip N).
- `DEPTH_WRITE` / `DEPTH_READ` must cover all planes (a DSV binds both).

### Compiling and executing
- **Barriers are fully computed at compile time and don't use any hazard tracking.** `Compile()` tracks the accesses per subresource. Each subresource that changes access (or is written) gets a barrier with an explicit source and destination access; these are merged per access into contiguous ranges with the same source (mips, then slices, then planes), so `myBarriers` already holds the optimized set. Backend: `CommandList::TransitionResourceForAccess( resource, range, srcAccess, dstAccess )`, which neither reads nor writes tracked state.
- **Boundary to everything outside the graph:** the graph's list has no hazard data, and the queue skips its submit-time patching for lists without hazard tracking. `Execute()` first submits a small tracked list (`CommandList::TrackResourceAccess()`) that moves each used subresource into the state of its first access in the graph, through the regular legacy tracking. The compiled graph then has no barrier for first accesses, and ends with `myCompiledFinalBarriers` from each subresource's last access back to its first. The legacy tracked state is therefore correct before and after the graph without any reporting. No second tracked list is needed after the graph, since the closing barriers are part of the compiled graph. Graph-created resources won't need the handover (their first access discards).
- Views are keyed by (kind, range) and created over that range; SRVs/UAVs of multi-plane textures must be single-plane. Views live in the builder-wide `myCompiledViews` array (`kFgMaxNumResourceViewsPerFrame` = `kFgMaxNumResources` * 8), because most resources only have one or two.
- The declaration hash covers each access's base index, range and access type.
- Engine fix: a stencil-plane view of a depth-stencil format is placed in the uint bindless tables.

### Tests (in `Test_FrameGraph`, next to the quadrants)
- **Mips:** `fgMipChain`, 256×256 RGBA8, 4 mips, mip 0 a checkerboard upload. Three `DownsampleMipTask`s (compute) each read `mips.Mips( n - 1 )` and write `mips.Mips( n )`.
- **Planes:** `fgDepthStencil`, 256×256 `D_32FLOAT_S_8UI`. `DrawStencilPatternTask` writes depth and stencil (clear, then rectangles with different stencil refs). `CopyStencilTask` (compute) reads `depthStencil.Planes( 1 )` and writes `fgStencilCopy` (`R8_UINT`).
- **Display:** the blit task also reads the whole mip chain, the stencil copy and `depthStencil.Planes( 0 )`, and draws them next to the quadrants.

### Status (2026-11-07)
Implemented and running on DX12 (debug layer) and Vulkan (validation): 11 tasks, 17 views, compiled once. The display shows the quadrants, the 4 mips, the stencil copy (refs 64 / 128 / 255) and the depth plane. The only remaining message is the known one-time DX12 `CREATE_NOT_ZEROED` error for `fgQuadrants`.

Engine bugs found and fixed on the way:
- DX12 submit-time patch barriers used the flat subresource index as mip level (wrong for planes and array slices).
- DX12: shader-writable textures with initial data were created in the UAV layout while the tracker assumed copy-dest.
- The stencil reference of depth-stencil states was never applied (DX12 `OMSetStencilRef`, Vulkan dynamic state).
- Vulkan clears are transfer operations; on lists without hazard tracking they now transition themselves and restore the declared access.
- The FrameGraph's DSV path didn't set `myIsRenderTarget` (it created an SRV). Depth-stencil textures need `bIsDepthStencil` and `myIsRenderTarget`.
- A stencil-plane view goes into the uint bindless tables.

### Possible feature: snapshots (implemented and removed again, 2026-11-08)
Goal: read the *old* contents of a resource after a later write was declared, e.g. SSAO reading the G-buffer normals without decals while the decal pass is set up first:
```cpp
FgResourceHandle normalsNoDecals = builder.Snapshot( gbuffer.myNormals );  // the contents as they are now
DecalPass::Setup( builder, gbuffer.myAlbedo, gbuffer.myNormals );          // overwrites the normals
SSAOPass::Setup( builder, normalsNoDecals );                               // reads the normals without decals
```
- **Semantics:** a snapshot read depends on the writers at snapshot time, and the graph either reorders it before the later writers (no extra memory, no copy) or, for transient resources, renames the later writes into new memory (more memory and a copy for partial writes, but the reader can run in parallel, e.g. SSAO on async compute next to the decals). Registered external resources can only be reordered. If reordering is impossible, it's a cycle.
- **Why removed:** the first version did this in `Read()` / `Write()` (write counts per subresource, a log of writes after the first snapshot, a special read path). Too intrusive for no current use, and the rename option needs `Create()` and the aliasing allocator first.
- **Alternatives considered:**
  - *Setup order:* set up the reader before the writer. Free (in place), but no parallelism. This is what to do today.
  - *Explicit renaming (`WriteToNew()`):* the writer writes into a new resource. No builder changes beyond `Create()`, same memory as snapshot + rename, but the writer has to know about the later reader, and partial writes (decals, single mips) need a copy of the old contents first.
- **If brought back:** implement it in the build step only. `Snapshot()` records a marker in setup order; the replay in `BuildFrameGraph()` copies the per-subresource writer at the marker, and for snapshot reads adds edges to the writes between marker and read (or decides to rename). `Read()` / `Write()` stay untouched. Test idea: overwrite the depth-stencil after the stencil pattern and copy the stencil from a snapshot taken before; the stencil panel must still show the pattern (depth clear must be 1.0, DX12 warns on other clear values every frame).

### Next: concurrent writes
Writes always get a write-after-write barrier and edge. Non-overlapping concurrent writes (culling, indirect arguments) don't need them. Options: byte ranges for buffer handles (provable, setup-time ranges only) and/or an explicit access flag (e.g. `FG_ACCESS_CONCURRENT`) that drops the barrier and edge between consecutive flagged writes; the next unflagged access waits for all of them.

### Later: barrier batching across tasks
Compiled barriers are stored on the receiving task and flushed right before its work, so one barrier call per task. Batching the barriers of several independent tasks into one call (Activision: interleaving independent pipelines with one merged barrier) belongs to a future scheduler, which would hoist them at compile time when it orders independent tasks.

### Later: async compute
The per-subresource state is what async compute needs as well. Extension points: split barriers by previous (access, queue) per subresource; combine runs of reads into one read state; explicit source states in `FrameGraphBarrier`.

## Next phase: graph-created resources and memory aliasing (planned 2026-11-08)

Goal: tasks create their resources inside the graph. The graph computes the lifetimes from the compiled task order, places all graph-created ("transient") resources in a few large GPU memory pages, lets resources with disjoint lifetimes share (alias) the same memory, and adds the needed aliasing barriers. Registered external resources (loaded textures, default textures, the backbuffer) keep working unchanged and are never aliased.

**Lifetime rule:** graph-created resources never outlive the frame, except temporal (history) resources, which are explicitly marked, rotate between N copies and live in their own heap (see "Temporal resources").

### Setup API
```cpp
FgResourceHandle CreateTexture( uint aTaskIndex, const FgTextureDesc & aDesc, const char * aName );
FgResourceHandle CreateBuffer( uint aTaskIndex, const FgBufferDesc & aDesc, const char * aName );
```
- `CreateTemporalTexture()` / `CreateTemporalBuffer()` for history resources, see "Temporal resources".
- `FgTextureDesc`: dimension, width, height, depth or array size, mips, format, usage flags (render target, depth-stencil, shader-writable). `FgBufferDesc`: size, usage flags (shader-writable, raw/structured later). Small POD structs instead of `TextureProperties` (which carries a path string and CPU access settings that don't apply). Later: sizes relative to another resource.
- Returns a handle over all subresources, like `RegisterExisting()`. `aTaskIndex` names the task that creates the resource (like `Read()` / `Write()`, nothing applies implicitly to the task added last), so tasks can be added first and their resources created afterwards.
- Contents are undefined until written: the build replay asserts if the first access of a transient subresource is a read.
- `FrameGraphResourceBase` gets `myIsTransient`, the desc, and its own `SubresourceRange mySubresources` and `myIsTexture`, because a transient resource has no `GpuResource` yet during setup and build. Everything that currently asks `myResource->GetSubresources()` / `GetSubresourceIndex()` / `IsTexture()` switches to the base's copies (index computed with `GpuResource::CalcSubresourceIndex()`); `myResource` is only valid after `Compile()`.
- The declaration hash includes each transient's desc. Registered resources keep hashing their engine handle and generation.

### Compile: lifetimes and placement
Runs only when the hash changes, after the barriers are known:
1. **Lifetimes:** for each transient, the first and last position in `myTaskOrder` that accesses it.
2. **Memory requirements:** size, alignment and memory category per transient from the backend (see "Backend"), without creating the resource.
3. **Placement by simulation:** walk the task order; allocate each transient at its first use and free it after its last use. Freed blocks are reused by later allocations (first fit), which is exactly the aliasing. One simulation per memory category (buffers / non-RT textures / RT-DS textures: required by DX12 resource heap tier 1; tier 2 and Vulkan could merge them later).
4. **Physical memory:** after the simulation the page sizes are known; create one physical memory heap per page (or reuse the heaps of the previous compile if they're large enough, so toggling tasks doesn't reallocate).
5. **Resources:** create every transient as a placed resource at its page and offset; then views as today.
6. **Aliasing info:** for each transient, the previous occupant of its memory (if any) and that occupant's last access, used for the first barrier (below).

`ReleaseCompiledGraph()` destroys the transient resources (not the heaps, see "Heap sizing policy") after `WaitForIdle()`, as for views today. Temporal copies survive it; they're released when a compile no longer contains them.

### Temporal resources
For temporal accumulation (TAA, denoisers, accumulation in the path tracer), contents of previous frames must survive. Modelled after the Activision task graph (REAC 2023): temporal resources are first class, with automatic rotation between N copies.
```cpp
FgTemporalHandle history = builder.CreateTemporalTexture( desc, "TAA history", 2 );  // 2 layers: this frame and the previous one
FgResourceHandle current = builder.GetTemporalLayer( history, 0 );   // written this frame
FgResourceHandle previous = builder.GetTemporalLayer( history, 1 );  // last frame's contents (read only)
if ( builder.IsTemporalLayerValid( history, 1 ) ) { ... }            // false after creation, desc changes or a reset
```
- **Identity across frames:** declared every frame like everything else, matched by name. The builder keeps a persistent registry (not cleared by `Reset()`): name -> desc, layer count, N physical resources, rotation index, number of valid history frames.
- **Rotation:** at `Reset()` (frame start) the rotation index advances, so layer k maps to physical copy `(rotation - k) mod N`. Layer 0 is this frame's copy (contents undefined until written, like a transient); layers 1..N-1 are previous frames and can only be read.
- **History validity:** after creation, a desc change or `ResetTemporal( name )` (camera cut, teleport, resolution change), the number of valid history frames restarts at 0 and grows by one per frame up to N-1. Reading an invalid layer is allowed, but its contents are undefined; tasks check `IsTemporalLayerValid()` and fall back (e.g. no accumulation in the first frame). Split-screen later gets one registry entry per view.
- **Compiled graph:** each layer is its own base in the graph (layer 0 behaves like a transient without aliasing, older layers like registered resources). The physical copy behind a layer changes every frame without changing the hash, so tasks that access temporal layers are recompiled every frame, like backbuffer tasks today (`myAccessesBackBuffer` becomes "accesses a per-frame resource"). Views per physical copy are cached, so this only costs view lookups.
- **Barriers:** the physical copies persist across frames like registered resources, so the first access of each copy is patched from its global state at submit time, as today. Layer 0's first access discards (write after an undefined state); no aliasing barrier.
- **Own heap:** temporal copies are placed in separate persistent heaps (per memory category), never aliased with transients and never moved. They're allocated when a temporal resource first appears, and released (destroyed, memory freed) when a compile no longer contains it, i.e. it was removed from the DAG. A desc change reallocates that resource only.
- **Later optimization** (Activision's "temporal resource aliasing"): transients placed into the part of the frame where the current layer isn't live yet, rotating the transient layout with the copies. Out of scope for now.

### Heap sizing policy
- **Grow:** when a compile needs more than the existing pages, add pages (transients: the simulation's page list; temporals: the allocator adds a page when a new copy doesn't fit).
- **Shrink:** only when more than 128 MiB of a heap category are unused after a compile; otherwise keep the memory to avoid churn.
  - Transients: their resources are recreated on every compile anyway, so the heaps of that category are simply recreated at the required size.
  - Temporals: live copies are never moved (that would lose history). Only pages that became completely empty are released, and only once the free amount exceeds 128 MiB.

### Reusing PagedLinearAllocator
The simulation is a good fit for `PagedLinearAllocator`: virtual offsets over a list of pages, a free list with first-fit allocation, block merging on free, alignment handling, and an allocation that's larger than a page gets its own oversized page. Needed extensions:
- **Keep empty pages:** `Free()` currently destroys a page when its last allocation is freed, which during the simulation would drop and re-create pages and move offsets. Add a flag (constructor or member) that keeps empty pages, plus a way to release empty pages explicitly (needed by the temporal heaps for the 128 MiB shrink rule).
- **Temporal heaps** use the allocator with real page data (like `GpuMemoryAllocatorDX12` does: `CreatePageData()` / `DestroyPageData()` create and destroy the backend memory heaps), since their allocations persist across compiles.
- **`Reset()`:** drop all pages and free blocks for the next simulation.
- **No physical data during the simulation:** use the base class directly (its `CreatePageData()` does nothing); the frame graph creates the physical heaps afterwards from the final page list. The existing DX12 global allocators (`GpuMemoryAllocatorDX12`) are not used for transients.
- Page size: e.g. 64 MB (resources larger than that get an oversized page). Several pages instead of one huge block keep allocations manageable and let the heaps be reused partially.

### Backend (no FrameGraph references)
- `RenderCore::GetTextureMemoryRequirements( const TextureProperties & )` / `GetBufferMemoryRequirements( const GpuBufferProperties & )` → `{ size, alignment, GpuMemoryType category }`. DX12: `GetResourceAllocationInfo()` on the resource desc (the code exists inside `TextureDX12::Create()`, factor it out). Vulkan: `vkGetDeviceImageMemoryRequirements` / `vkGetDeviceBufferMemoryRequirements` (Vulkan 1.3 / maintenance4), or a temporary image.
- **Memory heaps:** `GpuMemoryHeapHandle RenderCore::CreateMemoryHeap( GpuMemoryType aCategory, uint64 aSize, const char * aName )` / `DeleteMemoryHeap()`. DX12: `ID3D12Heap` with the category's heap flags (`Adapter::ResolveHeapFlags()`). Vulkan: one `VkDeviceMemory` from the device-local memory type; creating a placed resource asserts that its `memoryTypeBits` contain that type.
- **Placed resources:** `RenderCore::CreateTexture( props, name, const GpuMemoryPlacement & )` / `CreateBuffer( ..., placement )` with `GpuMemoryPlacement { heap, offset }`. DX12: `CreatePlacedResource2` with the given heap and offset instead of allocating from the global allocator. Vulkan: `vkBindImageMemory` / `vkBindBufferMemory` at the offset instead of `vkAllocateMemory`. The resource remembers that it doesn't own its memory and doesn't free it in `Destroy()`.
- **Discard / aliasing barrier:** `TransitionResourceForAccess()` gets a variant (or flag) for the first access of a transient: contents are discarded, and the previous occupant's last access is the source scope.
  - DX12 (enhanced barriers): `LayoutBefore = UNDEFINED`, `AccessBefore = NO_ACCESS`, `SyncBefore` = sync of the previous occupant's last access (or `NONE` without one), `D3D12_TEXTURE_BARRIER_FLAG_DISCARD` for RT/DS textures. This also fixes the "must be initialized with Discard/Clear/Copy" error (`CREATE_NOT_ZEROED`).
  - Vulkan: `oldLayout = UNDEFINED`, `srcStageMask` / `srcAccessMask` from the previous occupant's last access.
  - Legacy tracking: the list's local hazard data marks the subresource's first state as "discarded", so the submit-time patching (`ResolveResourceHazardData()`) doesn't add a barrier from the stale global state; the last state is still written back.
- The DX12 "shader-writable texture with initial data" fix and the patch-barrier fix from the subresource phase are prerequisites (done).

### Async compute later
With several queues, two tasks on different queues between the same sync points run concurrently, so lifetimes must be extended to the sync points before aliasing. The simulation stays the same; only the lifetime ranges get wider.

### Memory viewer (fancy_imgui)
`FrameGraphMemoryViewer`, next to `FrameGraphViewer`, fed by a builder getter (`GetCompiledMemoryLayout()`: per page category, index and size; per transient base index, page, offset, size, first and last task position):
- **Temporal heaps:** shown as their own band; each copy spans the whole frame, labeled with its layer for this frame.
- **Timeline:** x-axis = tasks in execution order (names as column labels), y-axis = memory offset, one band per page (categories separated). Each transient is a rectangle `[first task, last task] × [offset, offset + size]`, colored per resource, name inside if it fits, tooltip with name, desc (size, format, mips), byte size and lifetime.
- **Live memory graph:** a small bar per task with the sum of live transient bytes; the peak marked.
- **Totals:** physical memory allocated (sum of pages), peak live, and the sum of all transient sizes without aliasing (the saving).
- Drawn with `ImDrawList` like the dependency viewer; external resources listed separately (not part of the timeline).

### Tests
The existing test switches to graph-created resources:
- Quadrant texture, quadrant color buffer, mip chain, depth-stencil and stencil copy become `CreateTexture()` / `CreateBuffer()` calls in the setup. Their descs replace the creation code in the test's constructor.
- Transient contents are undefined at the start of every frame:
  - quadrants: a new `ClearQuadrantsTask` (render target clear) before the fill tasks, so disabled quadrants show the clear color instead of old contents;
  - mip chain: mip 0 is filled by a copy (`COPY_SRC` / `COPY_DST`) from an **external** checkerboard texture created and uploaded by the test, which keeps the external path (plus copy accesses) under test. The backbuffer stays external too.
- "Recreate texture" becomes a size toggle of the quadrant texture (desc change → hash change → new placement), testing recompile and heap reuse.
- Temporal: a small accumulation demo, e.g. a 2-layer temporal texture where each frame blends a moving pattern into last frame's result (a visible trail). Checks rotation, history validity (the first frame has no history) and that a toggle removing the task releases the temporal copies.
- At least one aliasing case: the quadrant color buffer is dead after the last fill task, and a later buffer (e.g. a small scratch buffer for the stencil copy or a new "statistics" buffer) is created afterwards; the test asserts via `GetCompiledMemoryLayout()` that the two share memory, and the debug layers verify the aliasing barriers.
- DX12 debug layer and Vulkan validation must stay clean, and the `CREATE_NOT_ZEROED` error must be gone.

### Steps
1. `PagedLinearAllocator`: keep-empty-pages flag and `Reset()`.
2. Backend: memory requirements, memory heaps, placed textures and buffers (non-owning), discard/aliasing barrier, "discarded" first state in the local hazard data.
3. Builder: `CreateTexture()` / `CreateBuffer()`, base gets subresource range / texture flag / desc, hash, replay assert on undefined reads.
4. Compile: lifetimes, simulation per category, heaps (with reuse), placed resources, aliasing info and first-access barriers; `ReleaseCompiledGraph()`.
5. Temporal resources: registry, rotation, validity, persistent heaps, release on removal, per-frame recompile of their tasks.
6. Heap sizing policy (grow, shrink above 128 MiB free).
7. `FrameGraphMemoryViewer` (temporal heaps as their own band).
8. Test migration, the temporal demo and the aliasing check; run on DX12 and Vulkan.

### Open questions
- Decided (2026-11-08): transients never outlive the frame; history goes through temporal resources with their own heap; heaps shrink only above 128 MiB free.
- Temporal resources with subresources (e.g. a mip-mapped history): same handle narrowing as today, nothing special expected.
- Vulkan `bufferImageGranularity`: irrelevant as long as buffers and images are in different categories; revisit if categories get merged.

## Later phases
- **Subresources:** see "Next phase: subresources".
- **Validation:** a "validate only" mode for the legacy tracker (Debug builds) on graph lists, which asserts every transition the commands need was declared in the graph.
- **Resource creation:** see "Next phase: graph-created resources and memory aliasing".
- **Multiple queues:** one command list per queue, fence signal and wait for cross-queue edges, and Vulkan queue-family ownership transfers, which replace `SHARED_READ`.
- **Culling and optimization:** drop tasks whose outputs are never consumed, unless flagged as having side effects. Add split barriers and parallel recording.
- **Migrate PathTracer:** raster, RT, accumulation, sky LUTs and tonemapping.

## Follow-ups
- **Generate task definitions from macros.** Task classes will be generated from preprocessor templates, e.g.

  ```cpp
  BEGIN_TASK( MyTask )
    READ_TEXTURE( src )
  END_TASK()
  ```

  These generate the handle members, `RenderData` and `Compile()`, so a task only writes `Setup()` logic and `Execute()`.

## Cleanup at the end
- **Merge `ShaderResourceAccess` into `ResourceAccess`.** Both exist side by side for now. `ShaderResourceAccess` (SRV, UAV, RTAS) has no shader stage and is used by `CommandList::PrepareResourceShaderAccess()`, so a merge needs a stage at every call site or a stage-less variant of the shader accesses.

## Open questions
- **Undefined contents.** `RegisterExisting()` declares defined contents, which is what history resources (accumulation) need. How does the caller declare that a registered resource has undefined contents, so the graph can discard it? A flag on `RegisterExisting()`, or only with `Create()`?
- **Side issues in `PagedLinearAllocator`,** to handle separately:
  - `GpuMemoryAllocatorDX12` uses the unaligned block start as the heap offset;
  - `Free()` gets back `mySize` instead of the aligned size plus padding that was used;
  - the `Page *` pointers it returns point into a vector that is pushed to and erased from.
