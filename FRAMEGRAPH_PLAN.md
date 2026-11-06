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
    TonemapTask & task = FgAddTask< TonemapTask >( aBuilder, "Tonemap", CommandListType::Graphics );
    task.myDest = aBuilder.Write( aDest, RESOURCE_ACCESS_RENDER_TARGET );
    task.mySource = aBuilder.Read( aSource, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
    task.myExposure = anExposure;
    return task;
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
- **`void * AddTask( const FgTaskCreateInfo & )`** (not templated) allocates the task instance in the builder's task allocator and returns the uninitialized memory. The create info holds:
  - the name and the queue type;
  - size and alignment of the task and of its `RenderData`;
  - the type-erased `Execute` and `Compile` functions, which take `void *` and cast back to the task's own types.
- **`FrameGraphTaskHelper.h`** (optional) generates all of this for task classes with the usual layout:
  - `FgMakeTaskCreateInfo< T >( name, queueType )` fills the create info, with `static_assert`s for trivially destructible task and `RenderData`;
  - `FgExecuteTask< T >` / `FgCompileTask< T >` are the casting functions;
  - `FgAddTask< T >( builder, name, queueType )` adds the task and placement-news a default-constructed `T` into the returned memory.
  The builder itself has no templates, so later generated tasks (see "Follow-ups") can fill the create info without the helper.
- **`FrameGraphCompileContext`** resolves the task's handles in `Compile()`: `GetRtv()`, `GetDsv()`, `GetSrv()`, `GetUav()` for textures, `GetBufferSrv()`, `GetBufferUav()` for buffers, and `GetResource()`. It asserts that the task declared the access and that the view type matches the access, and creates views on demand.
- Compute tasks record their queue type but run on the single graphics command list for now, with barriers using compute sync scopes. Async compute comes later.

### Resource API

Inside a task's `Setup()`, with the handles the caller passed in:

| Call | Meaning |
|---|---|
| `Read( handle, access )` | Records a read of exactly that version and returns the handle. Asserts on a write access. |
| `Write( handle, access )` | The handle has to be the latest version of its resource (asserts otherwise, so each resource has one linear history). Records the write and returns version + 1. Asserts on a read access. |

Before the tasks are set up, the caller registers all external resources every frame: `RegisterExisting( TextureHandle )`, `RegisterExisting( GpuBufferHandle )` and `RegisterBackBuffer()`. The returned handles are passed on to the tasks' `Setup()` functions.

- The returned handle is version 0, produced before the graph, with defined contents. Registering the same resource twice asserts.
- The builder requires a `RenderOutput` (constructor parameter). Its current backbuffer has no engine handle (`RenderOutput` stores the swap chain textures directly) and is recognized by comparing the resource pointer with it. Its view comes from the `RenderOutput` every frame.
- For all other resources, `FrameGraphResourceBase` stores the engine handle's index and generation. Both go into the declaration hash, so a recreated resource (new generation, even at the same pool slot or address) recompiles the graph. The compiled graph keeps them per resource (`FrameGraphCompiledResource`) and asserts they still match when it hands out views.
- `ReleaseCompiledGraph()` has to be called before destroying a resource that the compiled graph has views of.

**`FgResourceHandle`:** the index of a version in `FrameGraphBuilder::GetResources()` and the builder's `myFrameGraphGenerationIndex` at creation. The generation index counts the builder's `Reset()` calls (i.e. frames), so a handle from a previous frame asserts.

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

All edges point from earlier to later setup order. The order comes from a Kahn sort, so it can differ from the setup order when there are independent tasks, e.g. a task that reads an outdated version may run before the task that wrote the newer version was set up. The order is still valid, since the write-after-read edges are in place.

### Compiling (only when the structure hash changes)

`Compile()` is its own step after `Setup()` and `BuildFrameGraph()`. Its first action is `ComputeDeclarationHash()`; the hash is not built up during setup. If it matches the compiled one, `Compile()` returns early.

**Hash over:**
- each task's name, queue type and execute thunk;
- each access's base index, resource (version) index and access type;
- each registered resource's type and engine handle (index and generation). The backbuffer has no handle and always contributes the same key.

**The output is a `FrameGraphCompiledGraph` with:**
- **Task order.**
- **One `RenderData` per task,** allocated in the compiled graph's own linear allocator and filled by the task's `Compile()`. Tasks that access the backbuffer are compiled again every frame, because the swap chain image changes.
- **Views,** created on demand and shared between tasks, over the whole subresource range. Each resource (`FrameGraphResourceBase` index) has its own list of at most `kFgMaxNumViewsPerResource` views in `myCompiledResources`, so finding a view is a bounded search over that resource's views only. A view is identified by its kind (SRV, UAV, RTV, DSV, read-only DSV); the format will be added once accesses can request one. Views are owned by the compiled graph and destroyed on recompile.
- **Member naming:** everything that belongs to the compiled graph is called `myCompiled...` and declared together in the builder.
- **Barrier lists per task.** Between two accesses inside the graph, the transition is known at compile time.
- **Boundary states per resource:** the state of its first access and of its last access. The backbuffer's last state is `PRESENT`.

### Executing (every frame)
1. Begin a graphics command list with hazard tracking turned off, and resolve the external resources.
2. For each task in order: issue its barrier batch through a new explicit barrier entry point on `CommandList`, then call `Execute()` with the current frame's task instance and its compiled `RenderData`.
3. Hand each resource's boundary states to the command list (see below) and submit it.

Task instances come from the current frame's `Setup()`, `RenderData` from the compiled graph, both indexed by task index.

### Hazard tracking per command list

**Replaces the program-wide switch:**
- Remove `RenderPlatformProperties::myEnableAutomaticHazardTracking`, `RenderCore::IsAutomaticHazardTrackingEnabled()` and `-noHazardTracking`.
- Instead, `RenderCore::BeginCommandList( type, flags )` takes an optional `CommandListFlags::NO_HAZARD_TRACKING` (or a bool), stored on the `CommandList`.
- The early return in `Track*SubresourceTransition()` checks that member.
- `RenderCore::BeginCommandList()` sets it every time, so a pooled list never carries it over.
- On a list without hazard tracking, `TransitionResource()` and `PrepareResourceShaderAccess()` assert: such lists only get explicit barriers through `TransitionResourceForAccess()`.
- I'd rather not do a scoped on/off within one list. Resources touched inside the scope would leave the legacy tracker's local state stale for the rest of the list.

**Boundaries with legacy lists:**
- The global hazard state is only updated when a list is submitted (`ResolveResourceHazardData()`). It isn't updated when the list is recorded.
- So the graph can't read the global state while recording. A legacy list recorded earlier but submitted later would invalidate it.
- Instead, the graph reuses the existing submit-time mechanism. For each resource, the explicit barrier entry point writes one entry into the list's local hazard data: `myFirstDstState` = the state of the first access, final state = the state of the last access.
- `ResolveResourceHazardData()` then patches the first transition from the global state and writes the final state back. This is exactly what it does today.
- Only the transitions inside the graph are recorded explicitly.

### Task memory

There's no suitable existing allocator:
- `SmallObjectAllocator<T>` is a pool for one type only. It's unused and broken: `( uint8 ) newPage` should be `( uint8 * )`, and its entries aren't aligned. It gets removed, together with `AlignedStorage.h`, which only it uses and which doesn't actually align anything.
- `PagedLinearAllocator` hands out offset ranges for GPU heaps and descriptor indices, not CPU memory.

Plan:
- Add a small linear arena, `Common/LinearAllocator.h`. It has a fixed-capacity, aligned byte buffer, `Allocate( size, alignment )` and `Reset()`.
- The builder holds two of capacity `kFgTaskMemorySize` (task instances and compiled `RenderData`). The containers of the graph have an inline capacity (`kFgMaxNumTasks`, `kFgMaxNumResources`, ...) but may overflow to the heap.
- Tasks and their `RenderData` must be trivially destructible (checked by `FrameGraphTaskHelper.h`), so `Reset()` just rewinds the arena.
- `AddTask()` returns `nullptr` without adding the task when the arena is full (after asserting).

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
   - boundary states written into the list's local hazard data, so the existing patching at submit time handles them.
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

**Barrier entry point:** `CommandList::TransitionResourceForAccess( const GpuResource *, ResourceAccess )`, in `CommandListDX12` and `CommandListVk`. Nothing in the backend refers to the FrameGraph. The platform `Track*` functions are split into a public wrapper, which returns early when the list has no hazard tracking, and a `*Internal` version that the graph uses.

**Engine fixes found by the test:**
- **DX12 pipeline state:** with an unchanged pipeline, a dispatch after a draw on the same list ran with the graphics pipeline still bound. Graphics, compute and raytracing share one pipeline-state slot, so applying one of them now marks the others as dirty.
- **`CommandList::SetClipRect()`:** the comment said x, y, width, height, but the code takes left, top, right, bottom.

**Differences from the design above:**
- **Barrier precision:** accesses map to the existing `GpuTextureState`/`GpuBufferState` values and Vulkan access masks and layouts. Per-stage precision stops at fragment-shader reads versus all other shader reads. Sync scopes per stage come later.
- **Barrier decisions:** a compiled barrier is issued whenever the access changes or it's a write. The existing `Track*Internal` logic decides the actual barrier. For example, DX12 adds a write-after-write barrier for UAV → UAV.
- **Same-access writes on Vulkan:** the tracker sees no state change for two writes in a row with the same access, and `ResourceUAVbarrier()` does nothing on Vulkan. `TransitionResourceForAccess()` therefore adds an explicit memory barrier (same access and layout on both sides) in that case.
- **Vulkan layouts:** `TransitionResourceForAccess()` uses the plain optimal layout of each access (e.g. `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`, `TRANSFER_SRC_OPTIMAL`) instead of `ResolveImageLayout()`, which reads the global hazard state at record time. That function only differs for resources in the `SHARED_READ` state, which the graph doesn't support until the multi-queue phase. `GENERAL` everywhere was rejected: it would lose depth and color compression.
- **No `PRESENT` access:** ImGui draws after the graph, and the present path already transitions the backbuffer.
- **Buffers:** buffer views are raw (`ByteAddressBuffer`) over the whole buffer. Structured and typed buffer views come later.
- **Recompiling:** a recompile waits for the graphics queue to go idle before deleting the old views.

**Next:**
- Discard the contents of a resource at its first access when they're known to be undefined (this needs a way to declare that, see "Open questions"): on DX12, an enhanced barrier from `LAYOUT_UNDEFINED` with `D3D12_TEXTURE_BARRIER_FLAG_DISCARD`; on Vulkan, `oldLayout = UNDEFINED`. This would fix the remaining DX12 error: the test texture is never written before its first use as a render target.

## Later phases
- **Subresources:** accesses carry a `SubresourceRange`, with overlap tests and per-subresource barrier state.
- **Validation:** a "validate only" mode for the legacy tracker (Debug builds) on graph lists, which asserts every transition the commands need was declared in the graph.
- **Resource creation:** `Create( desc )` creates resources inside the graph (contents undefined) in addition to `RegisterExisting()`. Lifetimes come from the compiled order, resources are allocated from a dedicated heap with aliasing, and aliasing barriers are added.
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
