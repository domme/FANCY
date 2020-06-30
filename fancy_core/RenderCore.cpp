#include "fancy_core_precompile.h"
#include "RenderCore.h"

#include "RenderCore_PlatformDX12.h"
#include "RenderCore_PlatformVk.h"

#include "DepthStencilState.h"
#include "ResourceRefs.h"
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "GpuReadbackBuffer.h"
#include "ShaderCompiler.h"
#include "FileWatcher.h"
#include "PathService.h"
#include "ShaderPipeline.h"
#include "Mesh.h"
#include "BlendState.h"
#include "Shader.h"
#include "RenderOutput.h"
#include "CommandList.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "TempResourcePool.h"
#include "GpuQueryHeap.h"
#include "TimeManager.h"
#include "TextureReadbackTask.h"
#include "CommandLine.h"
#include "TextureSampler.h"
#include "Material.h"

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
  }
//---------------------------------------------------------------------------//
  Slot<void(const ShaderPipeline*)> RenderCore::ourOnShaderPipelineRecompiled;
  bool RenderCore::ourDebugLogResourceBarriers = false;
  bool RenderCore::ourDebugWaitAfterEachSubmit = false;

  UniquePtr<RenderCore_Platform> RenderCore::ourPlatformImpl;
  UniquePtr<TempResourcePool> RenderCore::ourTempResourcePool;
  UniquePtr<FileWatcher> RenderCore::ourShaderFileWatcher;
  UniquePtr<ShaderCompiler> RenderCore::ourShaderCompiler;

  SharedPtr<DepthStencilState> RenderCore::ourDefaultDepthStencilState;
  SharedPtr<BlendState> RenderCore::ourDefaultBlendState;
  SharedPtr<Texture> RenderCore::ourDefaultDiffuseTexture;
  SharedPtr<Texture> RenderCore::ourDefaultNormalTexture;
  SharedPtr<Texture> RenderCore::ourDefaultSpecularTexture;

  std::map<uint64, SharedPtr<Shader>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<ShaderPipeline>> RenderCore::ourShaderPipelineCache;
  std::map<uint64, SharedPtr<BlendState>> RenderCore::ourBlendStateCache;
  std::map<uint64, SharedPtr<DepthStencilState>> RenderCore::ourDepthStencilStateCache;
  std::map<uint64, SharedPtr<TextureSampler>> RenderCore::ourSamplerCache;
  std::map<uint64, SharedPtr<VertexInputLayout>> RenderCore::ourVertexInputLayoutCache;
  
  DynamicArray<UniquePtr<GpuRingBuffer>> RenderCore::ourRingBufferPool;
  std::list<GpuRingBuffer*> RenderCore::ourAvailableRingBuffers;
  std::list<std::pair<uint64, GpuRingBuffer*>> RenderCore::ourUsedRingBuffers;

  DynamicArray<UniquePtr<GpuReadbackBuffer>> RenderCore::ourReadbackBuffers;

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

  DynamicArray<String> RenderCore::ourChangedShaderFiles;
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

  const ShaderCompiler* RenderCore::GetShaderCompiler()
  {
    return ourShaderCompiler.get();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init()
  {
    Init_0_Platform();
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

    UpdateChangedShaders();

    ourPlatformImpl->BeginFrame();

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
    if (ourDebugLogResourceBarriers)
      LOG_DEBUG("\n---Frame Begin---");
#endif
  }
//---------------------------------------------------------------------------//
  void RenderCore::EndFrame()
  {
    for (uint i = 0u; i < (uint)GpuQueryType::NUM; ++i)
      ASSERT(ourMappedQueryBufferData[i] == nullptr, "Open query readback detected at end of frame");

    ResolveUsedQueryData();

    CommandQueue* graphicsQueue = GetCommandQueue(CommandListType::Graphics);
    const uint64 completedFrameFence = graphicsQueue->GetLastRequestedFenceVal();

    ASSERT(!ourQueuedFrameDoneFences.IsFull());
    ourQueuedFrameDoneFences.Add(completedFrameFence);

    if (ourLastFrameDoneFences.IsFull())
      ourLastFrameDoneFences.RemoveFirstElement();
    ourLastFrameDoneFences.Add({ Time::ourFrameIdx, completedFrameFence });

    ourPlatformImpl->EndFrame();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    Shutdown_0_Resources();
    Shutdown_1_Services();
    Shutdown_2_Platform();
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
  CommandListType RenderCore::ResolveSupportedCommandListType(CommandListType aType)
  {
    const RenderPlatformCaps& caps = GetPlatformCaps();

    if (aType == CommandListType::Graphics
      || (aType == CommandListType::Compute && !caps.myHasAsyncCompute)
      || (aType == CommandListType::DMA && !caps.myHasAsyncCopy))
    {
      return CommandListType::Graphics;
    }

    return aType;
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12* RenderCore::GetPlatformDX12()
  {
#if FANCY_ENABLE_DX12
    return GetPlatformType() == RenderPlatformType::DX12 ? static_cast<RenderCore_PlatformDX12*>(ourPlatformImpl.get()) : nullptr;
#else
    return nullptr;
#endif
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformVk* RenderCore::GetPlatformVk()
  {
#if FANCY_ENABLE_VK
    return GetPlatformType() == RenderPlatformType::VULKAN ? static_cast<RenderCore_PlatformVk*>(ourPlatformImpl.get()) : nullptr;
#else
    return nullptr;
#endif
  }
//---------------------------------------------------------------------------//
  GpuRingBuffer* RenderCore::AllocateRingBuffer(CpuMemoryAccessType aCpuAccess, uint someBindFlags, uint64 aNeededByteSize, const char* aName /*= nullptr*/)
  {
    ASSERT(aCpuAccess != CpuMemoryAccessType::NO_CPU_ACCESS, "Ring buffers are expected to be either readable or writable from CPU");

    UpdateAvailableRingBuffers();

    for (auto it = ourAvailableRingBuffers.begin(); it != ourAvailableRingBuffers.end(); ++it)
    {
      GpuRingBuffer* buffer = *it;
      const GpuBufferProperties& bufferProps = buffer->GetBuffer()->GetProperties();
      if (buffer->GetBuffer()->GetByteSize() >= aNeededByteSize 
        && bufferProps.myCpuAccess == aCpuAccess
        && bufferProps.myBindFlags == someBindFlags)  // We could also re-use buffers with more general bind flags, but since this method is likely to be called with the same arguments each frame, it might be beneficial to use the best match all the time
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
    params.myBindFlags = someBindFlags;
    params.myCpuAccess = aCpuAccess;

    buf->Create(params, aName);
    ourRingBufferPool.push_back(std::move(buf));

    return ourRingBufferPool.back().get();
  }
//---------------------------------------------------------------------------//
  void RenderCore::ReleaseRingBuffer(GpuRingBuffer* aBuffer, uint64 aFenceVal)
  {
#if FANCY_RENDERER_USE_VALIDATION
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
  GpuBuffer* RenderCore::AllocateReadbackBuffer(uint64 aBlockSize, uint anOffsetAlignment, uint64& anOffsetToBlockOut)
  {
    for (UniquePtr<GpuReadbackBuffer>& readbackBuffer : ourReadbackBuffers)
    {
      uint64 offset;
      GpuBuffer* buffer = readbackBuffer->AllocateBlock(aBlockSize, anOffsetAlignment, offset);
      if (buffer != nullptr)
      {
        anOffsetToBlockOut = offset;
        return buffer;
      }
    }

    const uint64 newBufferSize = MathUtil::Align(aBlockSize, MathUtil::Align(2 * SIZE_MB, anOffsetAlignment));
#if FANCY_RENDERER_DEBUG
    LOG_INFO("Allocating new readback buffer of size %d", newBufferSize);
#endif // FANCY_RENDERER_DEBUG
    ourReadbackBuffers.push_back(std::make_unique<GpuReadbackBuffer>(newBufferSize));

    uint64 offset;
    GpuBuffer* buffer = ourReadbackBuffers.back()->AllocateBlock(aBlockSize, anOffsetAlignment, offset);
    ASSERT(buffer != nullptr);

    anOffsetToBlockOut = offset;
    return buffer;
  }
//---------------------------------------------------------------------------//
  void RenderCore::FreeReadbackBuffer(GpuBuffer* aBuffer, uint64 aBlockSize, uint64 anOffsetToBlock)
  {
    for (auto it = ourReadbackBuffers.begin(); it != ourReadbackBuffers.end(); ++it)
    {
      UniquePtr<GpuReadbackBuffer>& readbackBuffer = *it;
      if (readbackBuffer->FreeBlock(aBuffer, anOffsetToBlock, aBlockSize))
      {
        if (readbackBuffer->IsEmpty() && it != ourReadbackBuffers.begin())  // Always keep one readback buffer around
        {
#if FANCY_RENDERER_DEBUG
          LOG_INFO("Deleting readback buffer of size %d", readbackBuffer->GetFreeSize());
#endif // FANCY_RENDERER_DEBUG

          ourReadbackBuffers.erase(it);
        }

        return;
      }
    }

    ASSERT(false, "Readback-buffer allocation not found");
  }
//---------------------------------------------------------------------------//
  TextureReadbackTask RenderCore::ReadbackTexture(Texture* aTexture, const SubresourceRange& aSubresourceRange, CommandListType aCommandListType /*= CommandListType::Graphics*/)
  {
    const TextureProperties& texProps = aTexture->GetProperties();
    const DataFormatInfo& dataFormatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
    const RenderPlatformCaps& caps = GetPlatformCaps();

    uint64* alignedSubresourceSizes = static_cast<uint64*>(alloca(sizeof(uint64) * aSubresourceRange.GetNumSubresources()));

    uint64 requiredBufferSize = 0u;
    uint i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it)
    {
      const SubresourceLocation& subResource = *it;
      
      uint width, height, depth;
      texProps.GetSize(subResource.myMipLevel, width, height, depth);

      const uint64 rowSize = MathUtil::Align(width * dataFormatInfo.myCopyableSizePerPlane[subResource.myPlaneIndex], caps.myTextureRowAlignment);
      const uint64 subresourceSize = MathUtil::Align(rowSize * height * depth, (uint64) caps.myTextureSubresourceBufferAlignment);
      alignedSubresourceSizes[i++] = subresourceSize;
      
      requiredBufferSize += subresourceSize;
    }

    uint64 offsetToReadbackBuffer;
    GpuBuffer* readbackBuffer = AllocateReadbackBuffer(requiredBufferSize, caps.myTextureSubresourceBufferAlignment, offsetToReadbackBuffer);
    ASSERT(readbackBuffer != nullptr);

    CommandList* ctx = BeginCommandList(aCommandListType);

    uint64 dstOffset = offsetToReadbackBuffer;
    i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it)
    {
      const SubresourceLocation& subResource = *it;
      ctx->CopyTextureToBuffer(readbackBuffer, dstOffset, aTexture, subResource);
      dstOffset += alignedSubresourceSizes[i++];
    }

    const uint64 fence = ExecuteAndFreeCommandList(ctx);

    SharedPtr<ReadbackBufferAllocation> bufferAlloc(new ReadbackBufferAllocation);
    bufferAlloc->myBlockSize = requiredBufferSize;
    bufferAlloc->myOffsetToBlock = offsetToReadbackBuffer;
    bufferAlloc->myBuffer = readbackBuffer;

    return TextureReadbackTask(texProps, aSubresourceRange, bufferAlloc, fence);
  }
//---------------------------------------------------------------------------//
  ReadbackTask RenderCore::ReadbackBuffer(GpuBuffer* aBuffer, uint64 anOffset, uint64 aSize, CommandListType aCommandListType /*= CommandListType::Graphics*/)
  {
    uint64 offsetToReadbackBuffer;
    GpuBuffer* readbackBuffer = AllocateReadbackBuffer(aSize, 1u, offsetToReadbackBuffer);
    ASSERT(readbackBuffer != nullptr);

    CommandList* ctx = BeginCommandList(aCommandListType);
    
    ctx->CopyBuffer(readbackBuffer, offsetToReadbackBuffer, aBuffer, anOffset, aSize);
    
    const uint64 fence = ExecuteAndFreeCommandList(ctx);

    SharedPtr<ReadbackBufferAllocation> bufferAlloc(new ReadbackBufferAllocation);
    bufferAlloc->myBlockSize = aSize;
    bufferAlloc->myOffsetToBlock = offsetToReadbackBuffer;
    bufferAlloc->myBuffer = readbackBuffer;

    return ReadbackTask(bufferAlloc, fence);
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_0_Platform()
  {
    ASSERT(ourPlatformImpl == nullptr);

    const CommandLine* commandLine = CommandLine::GetInstance();

    RenderPlatformType platformType = RenderPlatformType::DX12;
    if (commandLine->HasArgument("vulkan"))
      platformType = RenderPlatformType::VULKAN;

    switch (platformType)
    {
      case RenderPlatformType::DX12:
#if FANCY_ENABLE_DX12
        ourPlatformImpl = std::make_unique<RenderCore_PlatformDX12>();
#else
        ASSERT(false, "DX12 not supported. Recompile with FANCY_ENABLE_DX12 1");
#endif
        break;
      case RenderPlatformType::VULKAN:
#if FANCY_ENABLE_VK
        ourPlatformImpl = std::make_unique<RenderCore_PlatformVk>();
#else
        ASSERT(false, "Vulkan not supported. Recompile with FANCY_ENABLE_VK 1");
#endif
        break;
      default:
        break;
    }
    ASSERT(ourPlatformImpl != nullptr, "Unsupported rendering API requested");

    ourCommandQueues[(uint)CommandListType::Graphics].reset(ourPlatformImpl->CreateCommandQueue(CommandListType::Graphics));
    if (GetPlatformCaps().myHasAsyncCopy)
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

    const CommandLine* cmdLine = CommandLine::GetInstance();
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
    ourDebugLogResourceBarriers = cmdLine->HasArgument("logBarriers");
#endif
    ourDebugWaitAfterEachSubmit = cmdLine->HasArgument("waitAfterSubmit");
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_2_Resources()
  {
    ASSERT(ourPlatformImpl != nullptr);

    {
      TextureProperties props;
      props.myFormat = DataFormat::SRGB_8_A_8;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myHeight = 1u;
      props.myWidth = 1u;
      props.path = TextureRef::ToString(TextureRef::DEFAULT_DIFFUSE);

      TextureSubData data(props);
      uint8 color[4] = { 0, 0, 0, 255 };
      data.myData = color;

      ourDefaultDiffuseTexture = CreateTexture(props, "Default_Diffuse", &data, 1);

      props.path = TextureRef::ToString(TextureRef::DEFAULT_SPECULAR);
      ourDefaultSpecularTexture = CreateTexture(props, "Default_Specular", &data, 1);
    }

    {
      TextureProperties props;
      props.myFormat = DataFormat::RGBA_8;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myHeight = 1u;
      props.myWidth = 1u;
      props.path = TextureRef::ToString(TextureRef::DEFAULT_NORMAL);

      TextureSubData data(props);
      uint8 color[4] = { 128, 128, 128, 255 };
      data.myData = color;

      ourDefaultNormalTexture = CreateTexture(props, "Default_Normal", &data, 1);
    }

    ourDefaultDepthStencilState = CreateDepthStencilState(DepthStencilStateProperties());
    ourDefaultBlendState = CreateBlendState(BlendStateProperties());

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
    for (uint i = 0u; i < NUM_QUERY_BUFFERS; ++i)
    {
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
      {
        bufferProps.myElementSizeBytes = GetQueryTypeDataSize((GpuQueryType)queryType);
        bufferProps.myNumElements = numQueriesPerType[queryType];
        String name(StaticString<64>("QueryHeap %s", locGetQueryTypeName((GpuQueryType)queryType)));

        GpuBuffer* buffer = ourPlatformImpl->CreateBuffer();
        if (buffer)
          buffer->Create(bufferProps, name.c_str());
        ourQueryBuffers[i][queryType].reset(buffer);
      }
    }

    ourReadbackBuffers.push_back(std::make_unique<GpuReadbackBuffer>(64 * SIZE_MB));
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

    ourShaderPipelineCache.clear();
    ourShaderCache.clear();
    ourBlendStateCache.clear();
    ourDepthStencilStateCache.clear();

    for (uint i = 0u; i < NUM_QUERY_BUFFERS; ++i)
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
        ourQueryBuffers[i][queryType].reset();

    for (uint i = 0u; i < NUM_QUEUED_FRAMES; ++i)
      for (uint queryType = 0u; queryType < (uint)GpuQueryType::NUM; ++queryType)
        ourQueryHeaps[i][queryType].reset();

    ourReadbackBuffers.clear();
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
  void RenderCore::UpdateChangedShaders()
  {
    StaticArray<Shader*, 32> shadersToRecompile;
    for (const String& shaderFile : ourChangedShaderFiles)
    {
      // Find GpuPrograms for this file
      for (auto it = ourShaderCache.begin(); it != ourShaderCache.end(); ++it)
      {
        Shader* program = it->second.get();

        const ShaderDesc& desc = program->GetDescription();
        String actualShaderPath =
          Path::GetAbsoluteResourcePath(ourShaderCompiler->GetShaderPathRelative(desc.myPath.c_str()));

        if (actualShaderPath == shaderFile)
          shadersToRecompile.Add(program);
      }
    }
    ourChangedShaderFiles.clear();

    for (uint i = 0u; i < shadersToRecompile.Size(); ++i)
    {
      Shader* shader = shadersToRecompile[i];

      ShaderCompilerResult compiledOutput;
      if (ourShaderCompiler->Compile(shader->GetDescription(), &compiledOutput))
        shader->SetFromCompilerOutput(compiledOutput);
      else
        LOG_WARNING("Failed compiling shader %s", shader->GetDescription().myPath.c_str());
    }

    // Check which pipelines need to be updated...
    StaticArray<ShaderPipeline*, 32> changedPipelines;
    for (auto it = ourShaderPipelineCache.begin(); it != ourShaderPipelineCache.end(); ++it)
    {
      ShaderPipeline* pipeline = it->second.get();

      for (uint i = 0u; i < shadersToRecompile.Size(); ++i)
      {
        Shader* changedShader = shadersToRecompile[i];
        const uint stage = static_cast<uint>(changedShader->myProperties.myShaderStage);
        if (changedShader == pipeline->GetShader(stage))
        {
          changedPipelines.Add(pipeline);
          break;
        }
      }
    }

    for (uint i = 0u; i < changedPipelines.Size(); ++i)
    {
      ShaderPipeline* pipeline = changedPipelines[i];
      pipeline->Recreate();
      ourOnShaderPipelineRecompiled(pipeline);
    }
  }
//---------------------------------------------------------------------------//
  SharedPtr<RenderOutput> RenderCore::CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
  {
    SharedPtr<RenderOutput> output(ourPlatformImpl->CreateRenderOutput(aNativeInstanceHandle, someWindowParams));
    return output;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Shader> RenderCore::CreateShader(const ShaderDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourShaderCache.find(hash);
    if (it != ourShaderCache.end())
      return it->second;

    ShaderCompilerResult compilerOutput;
    if (!ourShaderCompiler->Compile(aDesc, &compilerOutput))
      return nullptr;

    SharedPtr<Shader> program(ourPlatformImpl->CreateShader());
    program->SetFromCompilerOutput(compilerOutput);
    
    ourShaderCache.insert(std::make_pair(hash, program));

    const String actualShaderPath =
      Path::GetAbsoluteResourcePath(ourShaderCompiler->GetShaderPathRelative(aDesc.myPath.c_str()));

    ourShaderFileWatcher->AddFileWatch(actualShaderPath);

    return program;
  }
//---------------------------------------------------------------------------//
  SharedPtr<ShaderPipeline> RenderCore::CreateShaderPipeline(const ShaderPipelineDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourShaderPipelineCache.find(hash);
    if (it != ourShaderPipelineCache.end())
      return it->second;

    std::array<SharedPtr<Shader>, (uint)ShaderStage::NUM> pipelinePrograms{ nullptr };
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      if (!aDesc.myShader[i].myPath.empty())
        pipelinePrograms[i] = CreateShader(aDesc.myShader[i]);
    }

    SharedPtr<ShaderPipeline> pipeline(ourPlatformImpl->CreateShaderPipeline());
    pipeline->Create(pipelinePrograms);

    ourShaderPipelineCache.insert(std::make_pair(hash, pipeline));

    return pipeline;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Shader> RenderCore::GetShader(uint64 aDescHash)
  {
    auto it = ourShaderCache.find(aDescHash);
    if (it != ourShaderCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<ShaderPipeline> RenderCore::GetShaderPipeline(uint64 aDescHash)
  {
    auto it = ourShaderPipelineCache.find(aDescHash);
    if (it != ourShaderPipelineCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<BlendState> RenderCore::CreateBlendState(const BlendStateProperties& aProperties)
  {
    const uint64 hash = MathUtil::ByteHash(aProperties);

    auto it = ourBlendStateCache.find(hash);
    if (it != ourBlendStateCache.end())
      return it->second;

    SharedPtr<BlendState> blendState(new BlendState(aProperties));

    ourBlendStateCache.insert(std::make_pair(hash, blendState));
    return blendState;
  }
//---------------------------------------------------------------------------//
  SharedPtr<DepthStencilState> RenderCore::CreateDepthStencilState(const DepthStencilStateProperties& aDesc)
  {
    const uint64 hash = MathUtil::ByteHash(aDesc);

    auto it = ourDepthStencilStateCache.find(hash);
    if (it != ourDepthStencilStateCache.end())
      return it->second;

    SharedPtr<DepthStencilState> depthStencilState(new DepthStencilState(aDesc));
    ourDepthStencilStateCache.insert(std::make_pair(hash, depthStencilState));
    return depthStencilState;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureSampler> RenderCore::CreateTextureSampler(const TextureSamplerProperties& someProperties)
  {
    const uint64 hash = MathUtil::ByteHash(someProperties);

    auto it = ourSamplerCache.find(hash);
    if (it != ourSamplerCache.end())
      return it->second;

    SharedPtr<TextureSampler> sampler(ourPlatformImpl->CreateTextureSampler(someProperties));
    ourSamplerCache.insert(std::make_pair(hash, sampler));
    return sampler;
  }
//---------------------------------------------------------------------------//
  SharedPtr<VertexInputLayout> RenderCore::CreateVertexInputLayout(const VertexInputLayoutProperties& aDesc)
  {
    const uint64 hash = aDesc.GetHash();

    auto it = ourVertexInputLayoutCache.find(hash);
    if (it != ourVertexInputLayoutCache.end())
      return it->second;

    SharedPtr<VertexInputLayout> layout(new VertexInputLayout(aDesc));
    ourVertexInputLayoutCache.insert(std::make_pair(hash, layout));
    return layout;
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
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureProperties& someProperties, const char* aName /*= nullptr*/, TextureSubData* someUploadDatas, uint aNumUploadDatas)
  {
    SharedPtr<Texture> tex(ourPlatformImpl->CreateTexture());
    if (!tex)
      return nullptr;

    tex->Create(someProperties, aName, someUploadDatas, aNumUploadDatas);
    return tex->IsValid() ? tex : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBuffer> RenderCore::CreateBuffer(const GpuBufferProperties& someProperties, const char* aName /*= nullptr*/, const void* someInitialData /* = nullptr */)
  {
    SharedPtr<GpuBuffer> buffer(ourPlatformImpl->CreateBuffer());
    if (!buffer)
      return nullptr;

    buffer->Create(someProperties, aName, someInitialData);
    return buffer->IsValid() ? buffer : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> RenderCore::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aName /*= nullptr*/)
  {
    const TextureProperties& texProps = aTexture->GetProperties();
    TextureViewProperties viewProps = someProperties;
    viewProps.myFormat = viewProps.myFormat != DataFormat::UNKNOWN ? viewProps.myFormat : texProps.myFormat;
    viewProps.myDimension = viewProps.myDimension != GpuResourceDimension::UNKONWN ? viewProps.myDimension : texProps.myDimension;
    viewProps.mySubresourceRange.myNumMipLevels = glm::max(1u, glm::min(viewProps.mySubresourceRange.myNumMipLevels, texProps.myNumMipLevels));
    viewProps.mySubresourceRange.myNumArrayIndices = glm::max(1u, glm::min(viewProps.mySubresourceRange.myNumArrayIndices, texProps.GetArraySize() - viewProps.mySubresourceRange.myFirstArrayIndex));
    viewProps.myZSize = glm::max(1u, glm::min(viewProps.myZSize, texProps.GetDepthSize() - viewProps.myFirstZindex));
    
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(viewProps.myFormat);
    ASSERT(viewProps.mySubresourceRange.myFirstPlane < formatInfo.myNumPlanes);
    ASSERT(!viewProps.myIsShaderWritable || !viewProps.myIsRenderTarget, "UAV and RTV are mutually exclusive");

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
    if (queue != nullptr)
      queue->WaitForIdle();
  }
//---------------------------------------------------------------------------//
  CommandQueue* RenderCore::GetCommandQueue(CommandListType aType)
  {
    const CommandListType commandListType = ResolveSupportedCommandListType(aType);
    return ourCommandQueues[(uint)commandListType].get();
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
    ourChangedShaderFiles.push_back(aShaderFile);
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileDeletedMoved(const String& aShaderFile)
  {
  }
//---------------------------------------------------------------------------//
}



