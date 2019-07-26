#include "fancy_core_precompile.h"
#include "RenderCore.h"

#include "DepthStencilState.h"
#include "ResourceRefs.h"
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "GpuProgramCompiler.h"
#include "FileWatcher.h"
#include "PathService.h"
#include "GpuProgramPipeline.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "BlendState.h"
#include "GpuProgram.h"
#include "RenderCore_PlatformDX12.h"
#include "VertexInputLayout.h"
#include "RenderOutput.h"
#include "CommandList.h"
#include "MeshData.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "TempResourcePool.h"
#include "GpuQueryHeap.h"
#include "TimeManager.h"

#include <xxHash/xxhash.h>

//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  namespace
  {
    const char* locGetQueryTypeName(GpuQueryType aQueryType)
    {
      switch (aQueryType)
      {
      case GpuQueryType::TIMESTAMP: return "Timestamp";
      case GpuQueryType::OCCLUSION: return "Occlusion";
      case GpuQueryType::NUM:
      default: ASSERT(false); return "";
      }
    }

    uint64 locComputeHashFromVertexData(const MeshData* someMeshDatas, uint aNumMeshDatas)
    {
      XXH64_state_t* xxHashState = XXH64_createState();
      XXH64_reset(xxHashState, 0u);

      for (uint i = 0u; i < aNumMeshDatas; ++i)
      {
        const MeshData& meshData = someMeshDatas[i];
        XXH64_update(xxHashState, meshData.myVertexData.data(), DYN_ARRAY_BYTESIZE(meshData.myVertexData));
        XXH64_update(xxHashState, meshData.myIndexData.data(), DYN_ARRAY_BYTESIZE(meshData.myIndexData));
      }

      uint64 hash = XXH64_digest(xxHashState);
      XXH64_freeState(xxHashState);
      return hash;
    }

    const uint kReadbackBufferSizeIncrease = 2 * SIZE_MB;
  }
//---------------------------------------------------------------------------//
  namespace
  {
    
  }
//---------------------------------------------------------------------------//
  Slot<void(const GpuProgram*)> RenderCore::ourOnShaderRecompiled;

  UniquePtr<RenderCore_Platform> RenderCore::ourPlatformImpl;
  UniquePtr<TempResourcePool> RenderCore::ourTempResourcePool;
  UniquePtr<FileWatcher> RenderCore::ourShaderFileWatcher;
  UniquePtr<GpuProgramCompiler> RenderCore::ourShaderCompiler;

  SharedPtr<DepthStencilState> RenderCore::ourDefaultDepthStencilState;
  SharedPtr<BlendState> RenderCore::ourDefaultBlendState;
  SharedPtr<Texture> RenderCore::ourDefaultDiffuseTexture;
  SharedPtr<Texture> RenderCore::ourDefaultNormalTexture;
  SharedPtr<Texture> RenderCore::ourDefaultSpecularTexture;

  std::map<uint64, SharedPtr<GpuProgram>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<GpuProgramPipeline>> RenderCore::ourGpuProgramPipelineCache;
  std::map<uint64, SharedPtr<BlendState>> RenderCore::ourBlendStateCache;
  std::map<uint64, SharedPtr<DepthStencilState>> RenderCore::ourDepthStencilStateCache;
  
  DynamicArray<UniquePtr<GpuRingBuffer>> RenderCore::ourRingBufferPool;
  std::list<GpuRingBuffer*> RenderCore::ourAvailableRingBuffers;
  std::list<std::pair<uint64, GpuRingBuffer*>> RenderCore::ourUsedRingBuffers;

  UniquePtr<CommandQueue> RenderCore::ourCommandQueues[(uint)CommandListType::NUM];

  StaticCircularArray<uint64, RenderCore::NUM_QUEUED_FRAMES> RenderCore::ourQueuedFrameDoneFences;
  StaticCircularArray<std::pair<uint64, uint64>, 256> RenderCore::ourLastFrameDoneFences;

  UniquePtr<GpuQueryHeap> RenderCore::ourQueryHeaps[NUM_QUEUED_FRAMES][(uint)GpuQueryType::NUM];
  uint RenderCore::ourCurrQueryHeapIdx = 0;

  FixedArray<std::pair<uint, uint>, 512> RenderCore::ourUsedQueryRanges[(uint)GpuQueryType::NUM];
  uint RenderCore::ourNumUsedQueryRanges[(uint)GpuQueryType::NUM] = { 0u };

  UniquePtr<GpuBuffer> RenderCore::ourQueryBuffers[NUM_QUERY_BUFFERS][(uint)GpuQueryType::NUM];
  uint64 RenderCore::ourQueryBufferFrames[NUM_QUERY_BUFFERS] = { UINT64_MAX };
  uint RenderCore::ourCurrQueryBufferIdx = 0u;

  const uint8* RenderCore::ourMappedQueryBufferData[(uint)GpuQueryType::NUM] = { nullptr };
  uint RenderCore::ourMappedQueryBufferIdx[(uint)GpuQueryType::NUM] = { 0u };
//---------------------------------------------------------------------------//  
  bool RenderCore::IsInitialized()
  {
    return ourPlatformImpl != nullptr && ourPlatformImpl->IsInitialized();
  }

  const Texture* RenderCore::GetDefaultDiffuseTexture()
  {
    return ourDefaultDiffuseTexture.get();
  }

  const Texture* RenderCore::GetDefaultNormalTexture()
  {
    return ourDefaultNormalTexture.get();
  }

  const Texture* RenderCore::GetDefaultMaterialTexture()
  {
    return ourDefaultSpecularTexture.get();
  }

  const GpuProgramCompiler* RenderCore::GetGpuProgramCompiler()
  {
    return ourShaderCompiler.get();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init(RenderPlatformType aRenderingApi)
  {
    Init_0_Platform(aRenderingApi);
    Init_1_Services();
    Init_2_Resources();
  }
//---------------------------------------------------------------------------//
  void RenderCore::BeginFrame()
  {
    if (ourQueuedFrameDoneFences.IsFull())
    {
      CommandQueue* graphicsQueue = GetCommandQueue(CommandListType::Graphics);
      graphicsQueue->WaitForFence(ourQueuedFrameDoneFences[0]);
      ourQueuedFrameDoneFences.RemoveFirstElement();
    }

    ourTempResourcePool->Reset();

    ourCurrQueryHeapIdx = (ourCurrQueryHeapIdx + 1) % NUM_QUEUED_FRAMES;
    for (uint i = 0u; i < (uint)GpuQueryType::NUM; ++i)
    {
      const uint64 lastUsedFrame = ourQueryHeaps[ourCurrQueryHeapIdx][i]->myLastUsedFrame;
      ASSERT(lastUsedFrame == UINT64_MAX || IsFrameDone(lastUsedFrame));
      ourQueryHeaps[ourCurrQueryHeapIdx][i]->Reset(Time::ourFrameIdx);
      ourNumUsedQueryRanges[i] = 0u;
    }

    ourCurrQueryBufferIdx = (ourCurrQueryBufferIdx + 1) % NUM_QUERY_BUFFERS;
    ourQueryBufferFrames[ourCurrQueryBufferIdx] = Time::ourFrameIdx;
  }
//---------------------------------------------------------------------------//
  void RenderCore::EndFrame()
  {
    for (uint i = 0u; i < (uint)GpuQueryType::NUM; ++i)
      ASSERT(ourMappedQueryBufferData[i] == nullptr, "Open query readback detected at end of frame");

    ResolveUsedQueryData();

    CommandQueue* graphicsQueue = GetCommandQueue(CommandListType::Graphics);
    const uint64 completedFrameFence = graphicsQueue->SignalAndIncrementFence();

    ASSERT(!ourQueuedFrameDoneFences.IsFull());
    ourQueuedFrameDoneFences.Add(completedFrameFence);

    if (ourLastFrameDoneFences.IsFull())
      ourLastFrameDoneFences.RemoveFirstElement();
    ourLastFrameDoneFences.Add({ Time::ourFrameIdx, completedFrameFence });
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    Shutdown_0_Resources();
    Shutdown_1_Services();
    Shutdown_2_Platform();
  }
//---------------------------------------------------------------------------//
  const char* RenderCore::ResourceUsageStateToString(GpuResourceState aState)
  {
    switch (aState)
    {
      case GpuResourceState::COMMON: return "COMMON";
      case GpuResourceState::READ_INDIRECT_ARGUMENT: return "READ_INDIRECT_ARGUMENT";
      case GpuResourceState::READ_VERTEX_BUFFER: return "READ_VERTEX_BUFFER";
      case GpuResourceState::READ_INDEX_BUFFER: return "READ_INDEX_BUFFER";
      case GpuResourceState::READ_VERTEX_SHADER_CONSTANT_BUFFER: return "READ_VERTEX_SHADER_CONSTANT_BUFFER";
      case GpuResourceState::READ_VERTEX_SHADER_RESOURCE: return "READ_VERTEX_SHADER_RESOURCE";
      case GpuResourceState::READ_PIXEL_SHADER_CONSTANT_BUFFER: return "READ_PIXEL_SHADER_CONSTANT_BUFFER";
      case GpuResourceState::READ_PIXEL_SHADER_RESOURCE: return "READ_PIXEL_SHADER_RESOURCE";
      case GpuResourceState::READ_COMPUTE_SHADER_CONSTANT_BUFFER: return "READ_COMPUTE_SHADER_CONSTANT_BUFFER";
      case GpuResourceState::READ_COMPUTE_SHADER_RESOURCE: return "READ_COMPUTE_SHADER_RESOURCE";
      case GpuResourceState::READ_ANY_SHADER_CONSTANT_BUFFER: return "READ_ANY_SHADER_CONSTANT_BUFFER";
      case GpuResourceState::READ_ANY_SHADER_RESOURCE: return "READ_ANY_SHADER_RESOURCE";
      case GpuResourceState::READ_COPY_SOURCE: return "READ_COPY_SOURCE";
      case GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH: return "READ_ANY_SHADER_ALL_BUT_DEPTH";
      case GpuResourceState::READ_DEPTH: return "READ_DEPTH";
      case GpuResourceState::READ_PRESENT: return "READ_PRESENT";
      case GpuResourceState::WRITE_VERTEX_SHADER_UAV: return "WRITE_VERTEX_SHADER_UAV";
      case GpuResourceState::WRITE_PIXEL_SHADER_UAV: return "WRITE_PIXEL_SHADER_UAV";
      case GpuResourceState::WRITE_COMPUTE_SHADER_UAV: return "WRITE_COMPUTE_SHADER_UAV";
      case GpuResourceState::WRITE_ANY_SHADER_UAV: return "WRITE_ANY_SHADER_UAV";
      case GpuResourceState::WRITE_RENDER_TARGET: return "WRITE_RENDER_TARGET";
      case GpuResourceState::WRITE_COPY_DEST: return "WRITE_COPY_DEST";
      case GpuResourceState::WRITE_DEPTH: return "WRITE_DEPTH";
      case GpuResourceState::UNKNOWN: return "UNKNOWN";
      default: ASSERT(false); return "";
    }
  }
//---------------------------------------------------------------------------//
  const char* RenderCore::CommandListTypeToString(CommandListType aType)
  {
    switch(aType) 
    {
      case CommandListType::Graphics: return "Graphics";
      case CommandListType::Compute: return "Compute";
      case CommandListType::DMA: return "Copy";
      default: ASSERT(false); return "";
    }
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12* RenderCore::GetPlatformDX12()
  {
    return GetPlatformType() == RenderPlatformType::DX12 ? static_cast<RenderCore_PlatformDX12*>(ourPlatformImpl.get()) : nullptr;
  }
//---------------------------------------------------------------------------//
  GpuRingBuffer* RenderCore::AllocateRingBuffer(GpuBufferUsage aUsage, uint64 aNeededByteSize, const char* aName /*= nullptr*/)
  {
    UpdateAvailableRingBuffers();

    for (auto it = ourAvailableRingBuffers.begin(); it != ourAvailableRingBuffers.end(); ++it)
    {
      GpuRingBuffer* buffer = *it;
      if (buffer->GetBuffer()->GetByteSize() >= aNeededByteSize && buffer->GetBuffer()->GetProperties().myUsage == aUsage)
      {
        ourAvailableRingBuffers.erase(it);
        return buffer;
      }
    }

    // Create a new buffer
    std::unique_ptr<GpuRingBuffer> buf = std::make_unique<GpuRingBuffer>();

    GpuBufferProperties params;
    ASSERT(aNeededByteSize <= UINT_MAX, "Buffer size overflow. Consider making numElements 64 bit wide");
    params.myNumElements = aNeededByteSize;
    params.myElementSizeBytes = 1u;
    params.myUsage = aUsage;
    params.myCpuAccess = aUsage == GpuBufferUsage::STAGING_READBACK ? CpuMemoryAccessType::CPU_READ : CpuMemoryAccessType::CPU_WRITE;

    GpuResourceMapMode mapMode = GpuResourceMapMode::WRITE_UNSYNCHRONIZED;
    bool keepMapped = true;

    if (aUsage == GpuBufferUsage::STAGING_READBACK)
    {
      mapMode = GpuResourceMapMode::READ_UNSYNCHRONIZED;
      keepMapped = false;
    }

    buf->Create(params, mapMode, keepMapped, aName);
    ourRingBufferPool.push_back(std::move(buf));

    return ourRingBufferPool.back().get();
  }
//---------------------------------------------------------------------------//
  void RenderCore::ReleaseRingBuffer(GpuRingBuffer* aBuffer, uint64 aFenceVal)
  {
#if FANCY_RENDERER_HEAVY_VALIDATION
    auto predicate = [aBuffer](const std::pair<uint64, GpuRingBuffer*>& aPair) {
      return aPair.second == aBuffer;
    };
    ASSERT(std::find_if(ourUsedRingBuffers.begin(), ourUsedRingBuffers.end(), predicate) == ourUsedRingBuffers.end());
    ASSERT(std::find(ourAvailableRingBuffers.begin(), ourAvailableRingBuffers.end(), aBuffer) == ourAvailableRingBuffers.end());
#endif

    aBuffer->Reset();
    ourUsedRingBuffers.push_back(std::make_pair(aFenceVal, aBuffer));
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_0_Platform(RenderPlatformType aRenderingApi)
  {
    ASSERT(ourPlatformImpl == nullptr);

    switch (aRenderingApi)
    {
      case RenderPlatformType::DX12:
        ourPlatformImpl = std::make_unique<RenderCore_PlatformDX12>();
        break;
      case RenderPlatformType::VULKAN: break;
      default:;
    }
    ASSERT(ourPlatformImpl != nullptr, "Unsupported rendering API requested");

    ourPlatformImpl->InitCaps();

    ourCommandQueues[(uint)CommandListType::Graphics].reset(ourPlatformImpl->CreateCommandQueue(CommandListType::Graphics));
    ourCommandQueues[(uint)CommandListType::Compute].reset(ourPlatformImpl->CreateCommandQueue(CommandListType::Compute));

    // From here, resources can be created that depend on ourPlatformImpl
    ourPlatformImpl->InitInternalResources();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_1_Services()
  {
    ASSERT(ourPlatformImpl != nullptr);

    ourShaderFileWatcher = std::make_unique<FileWatcher>();
    std::function<void(const String&)> onUpdatedFn(&RenderCore::OnShaderFileUpdated);
    ourShaderFileWatcher->myOnFileUpdated.Connect(onUpdatedFn);

    std::function<void(const String&)> onDeletedFn(&RenderCore::OnShaderFileDeletedMoved);
    ourShaderFileWatcher->myOnFileDeletedMoved.Connect(onDeletedFn);

    ourShaderCompiler.reset(ourPlatformImpl->CreateShaderCompiler());

    {
      ShaderVertexInputLayout& modelVertexLayout = ShaderVertexInputLayout::ourDefaultModelLayout;
      modelVertexLayout.myVertexInputElements.clear();

      uint registerIndex = 0u;
      ShaderVertexInputElement* elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Position";
      elem->mySemantics = VertexSemantics::POSITION;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Normal";
      elem->mySemantics = VertexSemantics::NORMAL;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Tangent";
      elem->mySemantics = VertexSemantics::TANGENT;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Bitangent";
      elem->mySemantics = VertexSemantics::BITANGENT;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Texcoord";
      elem->mySemantics = VertexSemantics::TEXCOORD;
      elem->myFormat = DataFormat::RG_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 8;
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_2_Resources()
  {
    ASSERT(ourPlatformImpl != nullptr);

    {
      TextureProperties props;
      props.eFormat = DataFormat::SRGB_8_A_8;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myHeight = 1u;
      props.myWidth = 1u;
      props.path = TextureRef::ToString(TextureRef::DEFAULT_DIFFUSE);

      TextureSubData data(props);
      uint8 color[3] = { 0, 0, 0 };
      data.myData = color;

      ourDefaultDiffuseTexture = CreateTexture(props, "Default_Diffuse", &data, 1);

      props.path = TextureRef::ToString(TextureRef::DEFAULT_SPECULAR);
      ourDefaultSpecularTexture = CreateTexture(props, "Default_Specular", &data, 1);
    }

    {
      TextureProperties props;
      props.eFormat = DataFormat::RGBA_8;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myHeight = 1u;
      props.myWidth = 1u;
      props.path = TextureRef::ToString(TextureRef::DEFAULT_NORMAL);

      TextureSubData data(props);
      uint8 color[3] = { 128, 128, 128 };
      data.myData = color;

      ourDefaultNormalTexture = CreateTexture(props, "Default_Normal", &data, 1);
    }

    ourDefaultDepthStencilState = CreateDepthStencilState(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    ASSERT(ourDefaultDepthStencilState != nullptr);

    ourDefaultBlendState = CreateBlendState(BlendStateDesc::GetDefaultSolid());
    ASSERT(ourDefaultBlendState != nullptr);

    ourTempResourcePool.reset(new TempResourcePool);

    const uint numQueriesPerType[] = 
    {
      4096, // Timestamp
      2048  // Occlusion
    };
    static_assert(ARRAYSIZE(numQueriesPerType) == (uint)GpuQueryType::NUM, "Missing values for numQueriesPerType");

    for (uint i = 0u; i < NUM_QUEUED_FRAMES; ++i)
    {
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
        ourQueryHeaps[i][queryType].reset(ourPlatformImpl->CreateQueryHeap((GpuQueryType)queryType, numQueriesPerType[queryType]));
    }

    GpuBufferProperties bufferProps;
    bufferProps.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    bufferProps.myUsage = GpuBufferUsage::STAGING_READBACK;
    for (uint i = 0u; i < NUM_QUERY_BUFFERS; ++i)
    {
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
      {
        bufferProps.myElementSizeBytes = GetQueryTypeDataSize((GpuQueryType)queryType);
        bufferProps.myNumElements = numQueriesPerType[queryType];
        String name(StaticString<64>("QueryHeap %s", locGetQueryTypeName((GpuQueryType)queryType)));

        GpuBuffer* buffer = ourPlatformImpl->CreateBuffer();
        buffer->Create(bufferProps, name.c_str());
        ourQueryBuffers[i][queryType].reset(buffer);
      }
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown_0_Resources()
  {
    UpdateAvailableRingBuffers();
    ASSERT(ourRingBufferPool.size() == ourAvailableRingBuffers.size(), "There are still some ringbuffers in flight");
    ourAvailableRingBuffers.clear();
    ourRingBufferPool.clear();

    ourDefaultDiffuseTexture.reset();
    ourDefaultNormalTexture.reset();
    ourDefaultSpecularTexture.reset();
    
    ourDefaultDepthStencilState.reset();
    ourDefaultBlendState.reset();

    ourGpuProgramPipelineCache.clear();
    ourShaderCache.clear();
    ourBlendStateCache.clear();
    ourDepthStencilStateCache.clear();

    ASSERT(ourRingBufferPool.size() == ourAvailableRingBuffers.size(), "There are still some ringbuffers in flight");
    ourAvailableRingBuffers.clear();
    ourRingBufferPool.clear();
  }
//---------------------------------------------------------------------------//  
  void RenderCore::Shutdown_1_Services()
  {
    ourShaderFileWatcher.reset();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown_2_Platform()
  {
    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      if (ourCommandQueues[i] != nullptr)
        ourCommandQueues[i]->WaitForIdle();

    ourPlatformImpl->Shutdown();
    ourPlatformImpl.reset();
  }
//---------------------------------------------------------------------------//
  void RenderCore::UpdateAvailableRingBuffers()
  {
    auto it = ourUsedRingBuffers.begin();
    while (it != ourUsedRingBuffers.end())
    {
      uint64 fence = it->first;
      GpuRingBuffer* buffer = it->second;
      
      CommandQueue* queue = GetCommandQueue(fence);
      if (queue->IsFenceDone(fence))
      {
        it = ourUsedRingBuffers.erase(it);
        ourAvailableRingBuffers.push_back(buffer);
      }
      else
        ++it;
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::ResolveUsedQueryData()
  {
    bool hasAnyQueryData = false;
    for (uint numRanges : ourNumUsedQueryRanges)
      hasAnyQueryData |= numRanges > 0u;

    if (!hasAnyQueryData)
      return;

    for (uint queryType = 0u; !hasAnyQueryData && queryType < (uint)GpuQueryType::NUM; ++queryType)
    {
      if (ourNumUsedQueryRanges[queryType] > 0u)
        hasAnyQueryData = true;
    }

    if (hasAnyQueryData)
    {
      CommandList* commandList = BeginCommandList(CommandListType::Graphics);
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
      {
        if (ourNumUsedQueryRanges[queryType] == 0u)
          continue;

        std::pair<uint, uint>* mergedRanges = (std::pair<uint, uint>*) alloca(sizeof(std::pair<uint, uint>) * ourNumUsedQueryRanges[queryType]);
        uint numUsedMergedRanges = 0u;

        std::pair<uint, uint> currMergedRange = ourUsedQueryRanges[queryType][0];
        for (uint i = 1u; i < ourNumUsedQueryRanges[queryType]; ++i)
        {
          const std::pair<uint, uint>& range = ourUsedQueryRanges[queryType][i];
          if (range.first == currMergedRange.second)
          {
            currMergedRange.second = range.second;
          }
          else
          {
            mergedRanges[numUsedMergedRanges++] = currMergedRange;
            currMergedRange = range;
          }
        }
        mergedRanges[numUsedMergedRanges++] = currMergedRange;

        const GpuQueryHeap* heap = ourQueryHeaps[ourCurrQueryHeapIdx][queryType].get();
        const GpuBuffer* readbackBuffer = ourQueryBuffers[ourCurrQueryBufferIdx][queryType].get();
        const uint queryDataSize = GetQueryTypeDataSize((GpuQueryType)queryType);
        for (uint i = 0u; i < numUsedMergedRanges; ++i)
        {
          const std::pair<uint, uint>& mergedRange = mergedRanges[i];
          const uint numQueries = mergedRange.second - mergedRange.first;
          const uint64 offsetInBuffer = mergedRange.first * queryDataSize;
          commandList->CopyQueryDataToBuffer(heap, readbackBuffer, mergedRange.first, numQueries, offsetInBuffer);
        }
      }

      ExecuteAndFreeCommandList(commandList);
    }
  }
//---------------------------------------------------------------------------//
  SharedPtr<RenderOutput> RenderCore::CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
  {
    SharedPtr<RenderOutput> output(ourPlatformImpl->CreateRenderOutput(aNativeInstanceHandle, someWindowParams));
    return output;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgram> RenderCore::CreateGpuProgram(const GpuProgramDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourShaderCache.find(hash);
    if (it != ourShaderCache.end())
      return it->second;

    GpuProgramCompilerOutput compilerOutput;
    if (!ourShaderCompiler->Compile(aDesc, &compilerOutput))
      return nullptr;

    SharedPtr<GpuProgram> program(ourPlatformImpl->CreateGpuProgram());
    program->SetFromCompilerOutput(compilerOutput);
    
    ourShaderCache.insert(std::make_pair(hash, program));

    const String actualShaderPath =
      Resources::FindPath(ourShaderCompiler->ResolvePlatformShaderPath(aDesc.myShaderFileName));

    ourShaderFileWatcher->AddFileWatch(actualShaderPath);

    return program;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourGpuProgramPipelineCache.find(hash);
    if (it != ourGpuProgramPipelineCache.end())
      return it->second;

    std::array<SharedPtr<GpuProgram>, (uint)ShaderStage::NUM> pipelinePrograms{ nullptr };
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      if (!aDesc.myGpuPrograms[i].myShaderFileName.empty())
        pipelinePrograms[i] = CreateGpuProgram(aDesc.myGpuPrograms[i]);
    }

    SharedPtr<GpuProgramPipeline> pipeline(new GpuProgramPipeline);
    pipeline->SetFromShaders(pipelinePrograms);

    ourGpuProgramPipelineCache.insert(std::make_pair(hash, pipeline));

    return pipeline;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgram> RenderCore::GetGpuProgram(uint64 aDescHash)
  {
    auto it = ourShaderCache.find(aDescHash);
    if (it != ourShaderCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::GetGpuProgramPipeline(uint64 aDescHash)
  {
    auto it = ourGpuProgramPipelineCache.find(aDescHash);
    if (it != ourGpuProgramPipelineCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<BlendState> RenderCore::CreateBlendState(const BlendStateDesc& aDesc)
  {
    auto it = ourBlendStateCache.find(aDesc.GetHash());
    if (it != ourBlendStateCache.end())
      return it->second;

    SharedPtr<BlendState> blendState(FANCY_NEW(BlendState, MemoryCategory::GENERAL));
    blendState->SetFromDescription(aDesc);

    ourBlendStateCache.insert(std::make_pair(aDesc.GetHash(), blendState));
    return blendState;
  }
//---------------------------------------------------------------------------//
  SharedPtr<DepthStencilState> RenderCore::CreateDepthStencilState(const DepthStencilStateDesc& aDesc)
  {
    auto it = ourDepthStencilStateCache.find(aDesc.GetHash());
    if (it != ourDepthStencilStateCache.end())
      return it->second;

    SharedPtr<DepthStencilState> depthStencilState(FANCY_NEW(DepthStencilState, MemoryCategory::GENERAL));
    depthStencilState->SetFromDescription(aDesc);

    ourDepthStencilStateCache.insert(std::make_pair(aDesc.GetHash(), depthStencilState));
    return depthStencilState;
  }
//---------------------------------------------------------------------------//
  const SharedPtr<BlendState>& RenderCore::GetDefaultBlendState()
  {
    return ourDefaultBlendState;
  }
//---------------------------------------------------------------------------//
  const SharedPtr<DepthStencilState>& RenderCore::GetDefaultDepthStencilState()
  {
    return ourDefaultDepthStencilState;
  }
//---------------------------------------------------------------------------//
  RenderPlatformType RenderCore::GetPlatformType()
  {
    return ourPlatformImpl->GetType();
  }
//---------------------------------------------------------------------------//
  const RenderPlatformCaps& RenderCore::GetPlatformCaps()
  {
    return ourPlatformImpl->GetCaps();
  }
//---------------------------------------------------------------------------//
  RenderCore_Platform* RenderCore::GetPlatform()
  {
    return ourPlatformImpl.get();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> RenderCore::CreateMesh(const MeshDesc& aDesc, const MeshData* someMeshDatas, uint aNumMeshDatas)
  {
    DynamicArray<SharedPtr<GeometryData>> vGeometryDatas;
    for (uint i = 0u; i < aNumMeshDatas; ++i)
    {
      const MeshData& meshData = someMeshDatas[i];
      const GeometryVertexLayout& vertexLayout = meshData.myLayout;

      SharedPtr<GeometryData> pGeometryData (FANCY_NEW(GeometryData, MemoryCategory::GEOMETRY));

      // Construct the vertex buffer
      const uint8* ptrToVertexData = meshData.myVertexData.data();
      const uint64 numVertices = (meshData.myVertexData.size() * sizeof(uint8)) / vertexLayout.myStride;

      SharedPtr<GpuBuffer> vertexBuffer(ourPlatformImpl->CreateBuffer());

      GpuBufferProperties bufferParams;
      bufferParams.myUsage = GpuBufferUsage::VERTEX_BUFFER;
      bufferParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      bufferParams.myNumElements = numVertices;
      bufferParams.myElementSizeBytes = vertexLayout.myStride;

      String name = "VertexBuffer_Mesh_" + aDesc.myUniqueName;
      vertexBuffer->Create(bufferParams, name.c_str(), ptrToVertexData);
      pGeometryData->setVertexLayout(vertexLayout);
      pGeometryData->setVertexBuffer(vertexBuffer);

      // Construct the index buffer
      const uint8* ptrToIndexData = meshData.myIndexData.data();
      const uint64 numIndices = (meshData.myIndexData.size() * sizeof(uint8)) / sizeof(uint);

      SharedPtr<GpuBuffer> indexBuffer(ourPlatformImpl->CreateBuffer());

      GpuBufferProperties indexBufParams;
      indexBufParams.myUsage = GpuBufferUsage::INDEX_BUFFER;
      indexBufParams.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
      indexBufParams.myNumElements = numIndices;
      indexBufParams.myElementSizeBytes = sizeof(uint);

      name = "IndexBuffer_Mesh_" + aDesc.myUniqueName;
      indexBuffer->Create(indexBufParams, name.c_str(), ptrToIndexData);
      pGeometryData->setIndexBuffer(indexBuffer);

      vGeometryDatas.push_back(pGeometryData);
    }

    SharedPtr<Mesh> mesh(FANCY_NEW(Mesh, MemoryCategory::GEOMETRY));
    mesh->myGeometryDatas = vGeometryDatas;
    mesh->myDesc = aDesc;
    mesh->myVertexAndIndexHash = locComputeHashFromVertexData(someMeshDatas, aNumMeshDatas);

    return mesh;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureProperties& someProperties, const char* aName /*= nullptr*/, TextureSubData* someUploadDatas, uint aNumUploadDatas)
  {
    SharedPtr<Texture> tex(ourPlatformImpl->CreateTexture());
    tex->Create(someProperties, aName, someUploadDatas, aNumUploadDatas);
    return tex->IsValid() ? tex : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBuffer> RenderCore::CreateBuffer(const GpuBufferProperties& someProperties, const char* aName /*= nullptr*/, const void* someInitialData /* = nullptr */)
  {
    SharedPtr<GpuBuffer> buffer(ourPlatformImpl->CreateBuffer());
    buffer->Create(someProperties, aName, someInitialData);
    return buffer->IsValid() ? buffer : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> RenderCore::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aName /*= nullptr*/)
  {
    const TextureProperties& texProps = aTexture->GetProperties();
    TextureViewProperties viewProps = someProperties;
    viewProps.myFormat = viewProps.myFormat != DataFormat::UNKNOWN ? viewProps.myFormat : texProps.eFormat;
    viewProps.myDimension = viewProps.myDimension != GpuResourceDimension::UNKONWN ? viewProps.myDimension : texProps.myDimension;
    viewProps.myNumMipLevels = glm::max(1u, glm::min(viewProps.myNumMipLevels, texProps.myNumMipLevels));
    viewProps.myArraySize = glm::max(1u, glm::min(viewProps.myArraySize, texProps.GetArraySize() - viewProps.myFirstArrayIndex));
    viewProps.myZSize = glm::max(1u, glm::min(viewProps.myZSize, texProps.GetDepthSize() - viewProps.myFirstZindex));
    
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(viewProps.myFormat);
    ASSERT(viewProps.myPlaneIndex < formatInfo.myNumPlanes);
    ASSERT(!viewProps.myIsShaderWritable || !viewProps.myIsRenderTarget, "UAV and RTV are mutually exclusive");
    ASSERT(viewProps.myPlaneIndex < GpuResourceView::ourNumSupportedPlanes);

    return SharedPtr<TextureView>(ourPlatformImpl->CreateTextureView(aTexture, viewProps, aName));
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> RenderCore::CreateTextureView(const TextureProperties& someProperties, const TextureViewProperties& someViewProperties, const char* aName /*= nullptr*/, TextureSubData* someUploadDatas, uint aNumUploadDatas)
  {
    SharedPtr<Texture> texture = CreateTexture(someProperties, aName, someUploadDatas, aNumUploadDatas);
    if (texture == nullptr)
      return nullptr;

    return CreateTextureView(texture, someViewProperties, aName);
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBufferView> RenderCore::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, GpuBufferViewProperties someProperties, const char* aName /*=nullptr*/)
  {
    if (someProperties.mySize == UINT64_MAX)
      someProperties.mySize = aBuffer->GetByteSize() - someProperties.myOffset;

    const DataFormat format = someProperties.myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);

    ASSERT(aBuffer->GetByteSize() >= someProperties.myOffset + someProperties.mySize, "Invalid buffer range");
    ASSERT(!someProperties.myIsStructured || someProperties.myStructureSize > 0u, "Structured buffer views need a valid structure size");
    ASSERT(!someProperties.myIsStructured || !someProperties.myIsRaw, "Raw and structured buffer views are mutually exclusive");
    ASSERT(!someProperties.myIsShaderWritable || aBuffer->GetProperties().myIsShaderWritable, "A shader-writable buffer view requires a shader-writable buffer");
    ASSERT(!someProperties.myIsStructured || format == DataFormat::UNKNOWN, "Structured buffer views can't have a format");
    ASSERT(!someProperties.myIsRaw || format == DataFormat::UNKNOWN || format == DataFormat::R_32UI, "Raw buffer views can't have a format other than R32");

    return SharedPtr<GpuBufferView>(ourPlatformImpl->CreateBufferView(aBuffer, someProperties, aName));
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBufferView> RenderCore::CreateBufferView(const GpuBufferProperties& someParams, GpuBufferViewProperties someViewProperties, const char* aName /*= nullptr*/, const void* someInitialData)
  {
    SharedPtr<GpuBuffer> buffer = CreateBuffer(someParams, aName, someInitialData);
    if (buffer == nullptr)
      return nullptr;

    return CreateBufferView(buffer, someViewProperties, aName);
  }
//---------------------------------------------------------------------------//
  uint RenderCore::GetQueryTypeDataSize(GpuQueryType aType)
  {
    return ourPlatformImpl->GetQueryTypeDataSize(aType);
  }
//---------------------------------------------------------------------------//
  MappedTempBuffer RenderCore::ReadbackBufferData(const GpuBuffer* aBuffer, uint64 anOffset, uint64 aByteSize)
  {
    ASSERT(anOffset + aByteSize <= aBuffer->GetByteSize());

    if (aBuffer->GetProperties().myCpuAccess == CpuMemoryAccessType::CPU_READ)
      LOG_WARNING("ReadbackBufferData() called with a CPU-readable buffer. Its better to directly map the buffer to avoid uneccessary temp-copies");

    GpuBufferResourceProperties props;
    props.myBufferProperties.myNumElements = MathUtil::Align(aByteSize, kReadbackBufferSizeIncrease); // Reserve a bit more size to make it more likely this buffer can be re-used for other, bigger readbacks
    props.myBufferProperties.myElementSizeBytes = 1u;
    props.myBufferProperties.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    props.myBufferProperties.myUsage = GpuBufferUsage::STAGING_READBACK;
    props.myIsShaderResource = false;
    props.myIsShaderWritable = false;
    TempBufferResource readbackBuffer  = AllocateTempBuffer(props, 0u, "Temp readback buffer");
    ASSERT(readbackBuffer.myBuffer != nullptr);

    CommandList* ctx = BeginCommandList(CommandListType::Graphics);
    ctx->CopyBufferRegion(readbackBuffer.myBuffer, 0u, aBuffer, anOffset, aByteSize);
    ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);

    return MappedTempBuffer(readbackBuffer, GpuResourceMapMode::READ, aByteSize);
  }
//---------------------------------------------------------------------------//
  MappedTempTextureBuffer RenderCore::ReadbackTextureData(const Texture* aTexture, const TextureSubLocation& aStartSubLocation, uint aNumSublocations)
  {
    DynamicArray<TextureSubLayout> subresourceLayouts;
    DynamicArray<uint64> subresourceOffsets;
    uint64 totalSize;
    aTexture->GetSubresourceLayout(aStartSubLocation, aNumSublocations, subresourceLayouts, subresourceOffsets, totalSize);

    GpuBufferResourceProperties props;
    props.myBufferProperties.myNumElements = MathUtil::Align(totalSize, kReadbackBufferSizeIncrease); // Reserve a bit more size to make it more likely this buffer can be re-used for other, bigger readbacks
    props.myBufferProperties.myElementSizeBytes = 1u;
    props.myBufferProperties.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    props.myBufferProperties.myUsage = GpuBufferUsage::STAGING_READBACK;
    props.myIsShaderResource = false;
    props.myIsShaderWritable = false;
    TempBufferResource readbackBuffer = AllocateTempBuffer(props, 0u, "Temp texture readback buffer");
    ASSERT(readbackBuffer.myBuffer != nullptr);

    const uint startSubresourceIndex = aTexture->GetSubresourceIndex(aStartSubLocation);

    CommandList* ctx = BeginCommandList(CommandListType::Graphics);
    for (uint subresource = 0; subresource < aNumSublocations; ++subresource)
    {
      TextureSubLocation subLocation = aTexture->GetSubresourceLocation(startSubresourceIndex + subresource);
      uint64 offset = subresourceOffsets[subresource];
      ctx->CopyTextureRegion(readbackBuffer.myBuffer, offset, aTexture, subLocation);
    }
    ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);

    return MappedTempTextureBuffer(subresourceLayouts, readbackBuffer, GpuResourceMapMode::READ, totalSize);
  }
//---------------------------------------------------------------------------//
  bool RenderCore::ReadbackTextureData(const Texture* aTexture, const TextureSubLocation& aStartSubLocation, uint aNumSublocations, TextureData& aTextureDataOut)
  {
    MappedTempTextureBuffer readbackTex = ReadbackTextureData(aTexture, aStartSubLocation, aNumSublocations);
    if (readbackTex.myMappedData == nullptr || readbackTex.myLayouts.empty())
      return false;

    const uint64 pixelSize = readbackTex.myLayouts.front().myRowSize / readbackTex.myLayouts.front().myWidth;

    uint64 totalSize = 0u;
    for (const TextureSubLayout& subLayout : readbackTex.myLayouts)
      totalSize += subLayout.myWidth * subLayout.myHeight * subLayout.myDepth * pixelSize;

    aTextureDataOut.myData.resize(totalSize);
    aTextureDataOut.mySubDatas.resize(readbackTex.myLayouts.size());

    uint8* dstSubResourceStart = aTextureDataOut.myData.data();
    const uint8* srcSubResourceStart = static_cast<const uint8*>(readbackTex.myMappedData);
    for (uint iSubResource = 0u, e = (uint) readbackTex.myLayouts.size(); iSubResource < e; ++iSubResource)
    {
      const TextureSubLayout& subLayout = readbackTex.myLayouts[iSubResource];

      TextureSubData& dstSubData = aTextureDataOut.mySubDatas[iSubResource];
      dstSubData.myData = dstSubResourceStart;
      dstSubData.myPixelSizeBytes = pixelSize;
      dstSubData.myRowSizeBytes = subLayout.myRowSize;
      dstSubData.mySliceSizeBytes = subLayout.myRowSize * subLayout.myHeight;
      dstSubData.myTotalSizeBytes = dstSubData.mySliceSizeBytes * subLayout.myDepth;

      for (uint iDepthSlice = 0u; iDepthSlice < subLayout.myDepth; ++iDepthSlice)
      {
        uint8* dstSliceStart = dstSubResourceStart + iDepthSlice * subLayout.myRowSize * subLayout.myHeight;
        const uint8* srcSliceStart = srcSubResourceStart + iDepthSlice * subLayout.myAlignedRowSize * subLayout.myNumRows;
        for (uint y = 0u; y < subLayout.myHeight; ++y)
        {
          uint8* dstRowStart = dstSliceStart + y * subLayout.myRowSize;
          const uint8* srcRowStart = srcSliceStart + y * subLayout.myAlignedRowSize;
          memcpy(dstRowStart, srcRowStart, subLayout.myRowSize);
        }
      }

      const uint64 dstSubresourceSize = subLayout.myWidth * subLayout.myHeight * subLayout.myDepth * pixelSize;
      dstSubResourceStart += dstSubresourceSize;

      const uint64 srcSubresourceSize = subLayout.myAlignedRowSize * subLayout.myNumRows * subLayout.myDepth;
      srcSubResourceStart += srcSubresourceSize;
    }

    return true;
  }
//---------------------------------------------------------------------------//
  CommandList* RenderCore::BeginCommandList(CommandListType aType)
  {
    ASSERT(aType == CommandListType::Graphics || aType == CommandListType::Compute,
      "CommandList type %d not implemented", (uint)aType);

    CommandQueue* queue = GetCommandQueue(aType);
    return queue->BeginCommandList();
  }
//---------------------------------------------------------------------------//
  uint64 RenderCore::ExecuteAndFreeCommandList(class CommandList* aCommandList, SyncMode aSyncMode)
  {
    CommandListType type = aCommandList->GetType();
    ASSERT(type == CommandListType::Graphics || type == CommandListType::Compute,
      "CommandList type %d not implemented", (uint)type);

    ASSERT(aCommandList->IsOpen(), "CommandList is not open (already executed?)");

    CommandQueue* queue = GetCommandQueue(aCommandList->GetType());
    return queue->ExecuteAndFreeCommandList(aCommandList, aSyncMode);
  }
//---------------------------------------------------------------------------//
  uint64 RenderCore::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    CommandListType type = aCommandList->GetType();
    ASSERT(type == CommandListType::Graphics || type == CommandListType::Compute,
      "CommandList type %d not implemented", (uint)type);

    ASSERT(aCommandList->IsOpen(), "CommandList is not open (already executed?)");

    CommandQueue* queue = GetCommandQueue(aCommandList->GetType());
    return queue->ExecuteAndResetCommandList(aCommandList, aSyncMode);
  }
//---------------------------------------------------------------------------//
  TempTextureResource RenderCore::AllocateTempTexture(const TextureResourceProperties& someProps, uint someFlags, const char* aName)
  {
    return ourTempResourcePool->AllocateTexture(someProps, someFlags, aName);
  }
//---------------------------------------------------------------------------//
  TempBufferResource RenderCore::AllocateTempBuffer(const GpuBufferResourceProperties& someProps, uint someFlags, const char* aName)
  {
    return ourTempResourcePool->AllocateBuffer(someProps, someFlags, aName);
  }
//---------------------------------------------------------------------------//
  uint RenderCore::AllocateQueryRange(GpuQueryType aType, uint aNumQueries)
  {
    GpuQueryHeap* heap = ourQueryHeaps[ourCurrQueryHeapIdx][(uint)aType].get();
    return heap->Allocate(aNumQueries);
  }
//---------------------------------------------------------------------------//
  void RenderCore::FreeQueryRange(GpuQueryType aType, uint aFirstQuery, uint aNumQueries, uint aNumUsedQueries)
  {
    GpuQueryHeap* heap = ourQueryHeaps[ourCurrQueryHeapIdx][(uint)aType].get();

    // Is this the last query-range that was allocated? Then deallocate the range in the storage again so it can be used again
    if (heap->myNextFreeQuery == aFirstQuery + aNumQueries)
      heap->myNextFreeQuery = aFirstQuery + aNumUsedQueries;

    if (aNumUsedQueries > 0u)
    {
      uint& numUsedQueryRanges = ourNumUsedQueryRanges[(uint)aType];
      ASSERT(numUsedQueryRanges < ourUsedQueryRanges[(uint)aType].size(), "Increase array-size of usedQueryRanges-array");
      ourUsedQueryRanges[(uint)aType][numUsedQueryRanges++] = std::make_pair(aFirstQuery, aFirstQuery + aNumUsedQueries);
    }
  }
//---------------------------------------------------------------------------//
  bool RenderCore::BeginQueryDataReadback(GpuQueryType aType, uint64 aFrameIdx, const uint8** aDataPtrOut /*= nullptr*/)
  {
    if(!IsFrameDone(aFrameIdx))
      return false;

    const uint type = (uint)aType;
    if (ourMappedQueryBufferData[type] != nullptr && ourQueryBufferFrames[ourMappedQueryBufferIdx[type]] == aFrameIdx)
    {
      if (aDataPtrOut != nullptr)
        *aDataPtrOut = ourMappedQueryBufferData[type];

      return true;
    }
    
    int bufferIdx = -1;
    for (uint i = 0u; bufferIdx < 0 && i < NUM_QUERY_BUFFERS; ++i)
    {
      if (ourQueryBufferFrames[i] == aFrameIdx)
        bufferIdx = i;
    }

    if (bufferIdx < 0)
      return false;

    GpuBuffer* buffer = ourQueryBuffers[bufferIdx][type].get();
    ourMappedQueryBufferData[type] = (const uint8*)buffer->Map(GpuResourceMapMode::READ_UNSYNCHRONIZED);
    ASSERT(ourMappedQueryBufferData[type] != nullptr);
    ourMappedQueryBufferIdx[type] = (uint) bufferIdx;

    if (aDataPtrOut != nullptr)
      *aDataPtrOut = ourMappedQueryBufferData[type];

    return true;
  }
//---------------------------------------------------------------------------//
  bool RenderCore::ReadQueryData(const GpuQuery& aQuery, uint8* aData)
  {
    const uint type = (uint) aQuery.myType;
    const uint8* mappedData = ourMappedQueryBufferData[type];
    if (mappedData == nullptr)
      return false;
    
    const uint bufferIdx = ourMappedQueryBufferIdx[type];
    if (aQuery.myFrame != ourQueryBufferFrames[bufferIdx])
      return false;

    const uint queryTypeDataSize = GetQueryTypeDataSize((GpuQueryType) type);
    const uint8 byteOffset = aQuery.myIndexInHeap * queryTypeDataSize;
    ASSERT(ourQueryBuffers[bufferIdx][type]->GetByteSize() >= byteOffset + queryTypeDataSize);

    memcpy(aData, mappedData + byteOffset, (size_t)queryTypeDataSize);
    return true;
  }
//---------------------------------------------------------------------------//
  void RenderCore::EndQueryDataReadback(GpuQueryType aType)
  {
    const uint type = (uint)aType;
    if (ourMappedQueryBufferData[type] == nullptr)
      return;

    const uint bufferIdx = ourMappedQueryBufferIdx[type];
    ourQueryBuffers[bufferIdx][type]->Unmap(GpuResourceMapMode::READ_UNSYNCHRONIZED);
    
    ourMappedQueryBufferData[type] = nullptr;
    ourMappedQueryBufferIdx[type] = 0u;
  }
//---------------------------------------------------------------------------//
  float64 RenderCore::GetGpuTicksToMsFactor(CommandListType aCommandListType)
  {
    return ourPlatformImpl->GetGpuTicksToMsFactor(aCommandListType);
  }
//---------------------------------------------------------------------------//
  bool RenderCore::IsFenceDone(uint64 aFenceVal)
  {
    CommandQueue* queue = GetCommandQueue(aFenceVal);
    ASSERT(queue);

    return queue->IsFenceDone(aFenceVal);
  }
//---------------------------------------------------------------------------//
  bool RenderCore::IsFrameDone(uint64 aFrameIdx)
  {
    if (ourLastFrameDoneFences.IsEmpty() || ourLastFrameDoneFences[ourLastFrameDoneFences.Size() - 1].first < aFrameIdx)
      return false;

    CommandQueue* queue = GetCommandQueue(CommandListType::Graphics);
    for (uint i = 0u, e = ourLastFrameDoneFences.Size(); i < e; ++i)
      if (ourLastFrameDoneFences[i].first == aFrameIdx)
        return queue->IsFenceDone(ourLastFrameDoneFences[i].second);

    ASSERT(false);  // It should never reach this point
    return false;
  }
//---------------------------------------------------------------------------//
  void RenderCore::WaitForFrame(uint64 aFrameIdx)
  {
    if (ourLastFrameDoneFences.IsEmpty() || ourLastFrameDoneFences[ourLastFrameDoneFences.Size() - 1].first < aFrameIdx)
    {
      WaitForIdle(CommandListType::Graphics);
      return;
    }

    if (ourLastFrameDoneFences[0].first > aFrameIdx)
      return;

    CommandQueue* queue = GetCommandQueue(CommandListType::Graphics);
    for (uint i = 0u, e = ourLastFrameDoneFences.Size(); i < e; ++i)
    {
      if (ourLastFrameDoneFences[i].first == aFrameIdx)
      {
        WaitForFence(ourLastFrameDoneFences[i].second);
        break;
      }
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::WaitForFence(uint64 aFenceVal)
  {
    CommandQueue* queue = GetCommandQueue(aFenceVal);
    ASSERT(queue);

    queue->WaitForFence(aFenceVal);
  }
//---------------------------------------------------------------------------//
  void RenderCore::WaitForIdle(CommandListType aType)
  {
    CommandQueue* queue = GetCommandQueue(aType);
    queue->WaitForIdle();
  }
//---------------------------------------------------------------------------//
  void RenderCore::WaitForResourceIdle(const GpuResource* aResource, uint aSubresourceOffset, uint aNumSubresources)
  {
    /* TODO: 
     * This method just waits on all queues the resource has been used on, even if it hasn't been written to or has been written to a long time ago. 
     * Instead, the hazardData should include the fences after the last write-access for all queues so this method can wait on those fences instead.
     */
    const GpuResourceStateTracking& hazardData = aResource->myStateTracking;

    bool commandListNeedsWait[(uint)CommandListType::NUM] = { true, true, false };
    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      if (commandListNeedsWait[i])
        WaitForIdle((CommandListType)i);
  }
//---------------------------------------------------------------------------//
  CommandQueue* RenderCore::GetCommandQueue(CommandListType aType)
  {
    return ourCommandQueues[(uint)aType].get();
  }
//---------------------------------------------------------------------------//
  CommandQueue* RenderCore::GetCommandQueue(uint64 aFenceVal)
  {
    CommandListType type = CommandQueue::GetCommandListType(aFenceVal);
    return GetCommandQueue(type);
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileUpdated(const String& aShaderFile)
  {
    // Find GpuPrograms for this file
    std::vector<GpuProgram*> programsToRecompile;
    for (auto it = ourShaderCache.begin(); it != ourShaderCache.end(); ++it)
    {
      GpuProgram* program = it->second.get();

      const GpuProgramDesc& desc = program->GetDescription();
      String actualShaderPath =
        Resources::FindPath(ourShaderCompiler->ResolvePlatformShaderPath(desc.myShaderFileName));

      if (actualShaderPath == aShaderFile)
        programsToRecompile.push_back(program);
    }
    
    for (GpuProgram* program : programsToRecompile)
    {
      GpuProgramCompilerOutput compiledOutput;
      if (ourShaderCompiler->Compile(program->GetDescription(), &compiledOutput))
        program->SetFromCompilerOutput(compiledOutput);
      else
        LOG_WARNING("Failed compiling shader %s", program->GetDescription().myShaderFileName.c_str());
    }
    
    // Check which pipelines need to be updated...
    std::vector<GpuProgramPipeline*> changedPipelines;
    for (auto it = ourGpuProgramPipelineCache.begin(); it != ourGpuProgramPipelineCache.end(); ++it)
    {
      GpuProgramPipeline* pipeline = it->second.get();

      for (GpuProgram* changedProgram : programsToRecompile)
      {
        const uint stage = static_cast<uint>(changedProgram->myProperties.myShaderStage);
        if (changedProgram == pipeline->myGpuPrograms[stage].get())
        {
          changedPipelines.push_back(pipeline);
          break;
        }
      }
    }

    for (GpuProgramPipeline* pipeline : changedPipelines)
    {
      pipeline->UpdateResourceInterface();
      pipeline->UpdateShaderByteCodeHash();
    }

    for (GpuProgram* program : programsToRecompile)
      ourOnShaderRecompiled(program);
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileDeletedMoved(const String& aShaderFile)
  {
  }
//---------------------------------------------------------------------------//
}



