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
#include "CommandContext.h"
#include "MeshData.h"
#include "TextureProperties.h"
#include "Texture.h"
#include "TempResourcePool.h"
#include "GpuQueryHeap.h"

#include <xxHash/xxhash.h>
#include "GpuQueryStorage.h"
#include "TimeManager.h"

//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Private_RenderCore
  {
    uint64 ComputeHashFromVertexData(const MeshData* someMeshDatas, uint aNumMeshDatas)
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

  DynamicArray<UniquePtr<CommandContext>> RenderCore::ourRenderContextPool;
  DynamicArray<UniquePtr<CommandContext>> RenderCore::ourComputeContextPool;
  std::list<CommandContext*> RenderCore::ourAvailableRenderContexts;
  std::list<CommandContext*> RenderCore::ourAvailableComputeContexts;
  
  DynamicArray<UniquePtr<GpuRingBuffer>> RenderCore::ourRingBufferPool;
  std::list<GpuRingBuffer*> RenderCore::ourAvailableRingBuffers;
  std::list<std::pair<uint64, GpuRingBuffer*>> RenderCore::ourUsedRingBuffers;
   
  StaticCircularArray<uint64, RenderCore::kMaxNumQueuedFrames> RenderCore::ourQueuedFrameDoneFences;
  StaticCircularArray<std::pair<uint64, uint64>, RenderCore::kNumLastFrameFences> RenderCore::ourLastFrameDoneFences;

  DynamicArray<glm::uvec2> RenderCore::ourUsedQueryRanges[(uint)GpuQueryType::NUM];
  GpuQueryStorage RenderCore::ourQueryStorages[kNumQueryStorages][(uint)GpuQueryType::NUM];
  uint64 RenderCore::ourQueryStorageLastUsedFrame[kNumQueryStorages] = { 0u };
  uint RenderCore::ourCurrQueryStorageIdx = 0u;
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
  void RenderCore::Init(RenderingApi aRenderingApi)
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

    for (DynamicArray<glm::uvec2>& usedQueryRange : ourUsedQueryRanges)
      usedQueryRange.clear();

    ASSERT(IsFrameDone(ourQueryStorageLastUsedFrame[ourCurrQueryStorageIdx]));
    for (uint i = 0u; i < (uint) GpuQueryType::NUM; ++i)
      ourQueryStorages[ourCurrQueryStorageIdx][i].FreeAllQueries();
  }
//---------------------------------------------------------------------------//
  void RenderCore::EndFrame()
  {
    ResolveUsedQueryData();

    CommandQueue* graphicsQueue = GetCommandQueue(CommandListType::Graphics);
    const uint64 completedFrameFence = graphicsQueue->SignalAndIncrementFence();

    ASSERT(!ourQueuedFrameDoneFences.IsFull());
    ourQueuedFrameDoneFences.Add(completedFrameFence);

    if (ourLastFrameDoneFences.IsFull())
      ourLastFrameDoneFences.RemoveFirstElement();
    ourLastFrameDoneFences.Add({ Time::ourFrameIdx, completedFrameFence });

    ourQueryStorageLastUsedFrame[ourCurrQueryStorageIdx] = Time::ourFrameIdx;
    ourCurrQueryStorageIdx = (ourCurrQueryStorageIdx + 1) % kNumQueryStorages;
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    Shutdown_0_Resources();
    Shutdown_1_Services();
    Shutdown_2_Platform();
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12* RenderCore::GetPlatformDX12()
  {
    return static_cast<RenderCore_PlatformDX12*>(ourPlatformImpl.get());
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
  void RenderCore::Init_0_Platform(RenderingApi aRenderingApi)
  {
    ASSERT(ourPlatformImpl == nullptr);

    switch (aRenderingApi)
    {
      case RenderingApi::DX12:
        ourPlatformImpl = std::make_unique<RenderCore_PlatformDX12>();
        break;
      case RenderingApi::VULKAN: break;
      default:;
    }
    ASSERT(ourPlatformImpl != nullptr, "Unsupported rendering API requested");

    ourPlatformImpl->InitCaps();

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

    for (uint storageIdx = 0u; storageIdx < kNumQueryStorages; ++storageIdx)
    {
      ourQueryStorages[storageIdx][(uint)GpuQueryType::OCCLUSION].Create(GpuQueryType::OCCLUSION, 2048);
      ourQueryStorages[storageIdx][(uint)GpuQueryType::TIMESTAMP].Create(GpuQueryType::TIMESTAMP, 4096);
    }
    ourCurrQueryStorageIdx = 0u;
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

    ASSERT(ourRenderContextPool.size() == ourAvailableRenderContexts.size(), "There are still some rendercontexts in flight");
    ourAvailableRenderContexts.clear();
    ourRenderContextPool.clear();

    ASSERT(ourComputeContextPool.size() == ourAvailableComputeContexts.size(), "There are still some compute contexts in flight");
    ourAvailableComputeContexts.clear();
    ourComputeContextPool.clear();

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
    for (const DynamicArray<glm::uvec2>& ranges : ourUsedQueryRanges)
      hasAnyQueryData |= !ranges.empty();

    if (!hasAnyQueryData)
      return;

    CommandQueue* queueGraphics = GetCommandQueue(CommandListType::Graphics);
    CommandContext* context = AllocateContext(CommandListType::Graphics);
    DynamicArray<glm::uvec2> mergedRanges;
    for (uint queryType = 0u; queryType < (uint) GpuQueryType::NUM; ++queryType)
    {
      DynamicArray<glm::uvec2>& ranges = ourUsedQueryRanges[queryType];
      if (ranges.empty())
        continue;

      hasAnyQueryData = true;
      mergedRanges.resize(0u);
      mergedRanges.reserve(ranges.size());

      glm::uvec2 currMergedRange = ranges.front();
      for (uint i = 1u, e = (uint) ranges.size(); i < e; ++i)
      {
        const glm::uvec2& range = ranges[i];
        if (range.x == currMergedRange.y)
        {
          currMergedRange.y = range.y;
        }
        else
        {
          mergedRanges.push_back(currMergedRange);
          currMergedRange = range;
        }
      }
      mergedRanges.push_back(currMergedRange);

      GpuQueryStorage& storage = ourQueryStorages[ourCurrQueryStorageIdx][queryType];
      const GpuQueryHeap* heap = storage.myQueryHeap.get();
      const GpuBuffer* readbackBuffer = storage.myReadbackBuffer.get();
      const uint queryDataSize = GetQueryTypeDataSize((GpuQueryType)queryType);
      uint64 bufferOffset = 0u;
      for (const glm::uvec2& mergedRange : mergedRanges)
      {
        const uint numQueries = mergedRange.y - mergedRange.x;
        context->CopyQueryDataToBuffer(heap, readbackBuffer, mergedRange.x, numQueries, bufferOffset);
        bufferOffset += static_cast<uint64>(numQueries * queryDataSize);
      }
    }

    if (hasAnyQueryData)
      queueGraphics->ExecuteContext(context);

    FreeContext(context);
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
    mesh->myVertexAndIndexHash = Private_RenderCore::ComputeHashFromVertexData(someMeshDatas, aNumMeshDatas);

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
  SharedPtr<TextureView> RenderCore::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
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

    return SharedPtr<TextureView>(ourPlatformImpl->CreateTextureView(aTexture, viewProps));
  }
//---------------------------------------------------------------------------//
  SharedPtr<TextureView> RenderCore::CreateTextureView(const TextureProperties& someProperties, const TextureViewProperties& someViewProperties, const char* aName /*= nullptr*/, TextureSubData* someUploadDatas, uint aNumUploadDatas)
  {
    SharedPtr<Texture> texture = CreateTexture(someProperties, aName, someUploadDatas, aNumUploadDatas);
    if (texture == nullptr)
      return nullptr;

    return CreateTextureView(texture, someViewProperties);
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBufferView> RenderCore::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
  {
    const DataFormat format = someProperties.myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);

    ASSERT(aBuffer->GetByteSize() >= someProperties.myOffset + someProperties.mySize, "Invalid buffer range");
    ASSERT(!someProperties.myIsStructured || someProperties.myStructureSize > 0u, "Structured buffer views need a valid structure size");
    ASSERT(!someProperties.myIsStructured || !someProperties.myIsRaw, "Raw and structured buffer views are mutually exclusive");
    ASSERT(!someProperties.myIsShaderWritable || aBuffer->GetProperties().myIsShaderWritable, "A shader-writable buffer view requires a shader-writable buffer");
    ASSERT(!someProperties.myIsStructured || format == DataFormat::UNKNOWN, "Structured buffer views can't have a format");
    ASSERT(!someProperties.myIsRaw || format == DataFormat::UNKNOWN || format == DataFormat::R_32UI, "Raw buffer views can't have a format other than R32");

    return SharedPtr<GpuBufferView>(ourPlatformImpl->CreateBufferView(aBuffer, someProperties));
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBufferView> RenderCore::CreateBufferView(const GpuBufferProperties& someParams, const GpuBufferViewProperties& someViewProperties, const char* aName /*= nullptr*/, const void* someInitialData)
  {
    SharedPtr<GpuBuffer> buffer = CreateBuffer(someParams, aName, someInitialData);
    if (buffer == nullptr)
      return nullptr;

    return CreateBufferView(buffer, someViewProperties);
  }
//---------------------------------------------------------------------------//
  uint RenderCore::GetQueryTypeDataSize(GpuQueryType aType)
  {
    return ourPlatformImpl->GetQueryTypeDataSize(aType);
  }
//---------------------------------------------------------------------------//
  // TODO: Add a parameter that decides about synchronized/unsynchronized access
  void RenderCore::UpdateBufferData(GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize)
  {
    ASSERT(aDestOffset + aByteSize <= aDestBuffer->GetByteSize());

    const GpuBufferProperties& bufParams = aDestBuffer->GetProperties();

    if (bufParams.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)
    {
      uint8* dest = static_cast<uint8*>(aDestBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, aDestOffset, aByteSize));
      ASSERT(dest != nullptr);
      memcpy(dest, aDataPtr, aByteSize);
      aDestBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, aDestOffset, aByteSize);
    }
    else
    {
      CommandContext* context = AllocateContext(CommandListType::Graphics);
      context->TransitionResource(aDestBuffer, GpuResourceTransition::TO_COPY_DEST);
      context->UpdateBufferData(aDestBuffer, aDestOffset, aDataPtr, aByteSize);
      GetCommandQueue(CommandListType::Graphics)->ExecuteContext(context, true);
      FreeContext(context);
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::UpdateTextureData(Texture* aDestTexture, const TextureSubLocation& aStartSubresource, const TextureSubData* someDatas, uint aNumDatas)
  {
    CommandContext* context = AllocateContext(CommandListType::Graphics);

    // TODO: Only transition the required subresources to COPY_DEST
    context->TransitionResource(aDestTexture, GpuResourceTransition::TO_COPY_DEST);
    context->UpdateTextureData(aDestTexture, aStartSubresource, someDatas, aNumDatas);
    GetCommandQueue(CommandListType::Graphics)->ExecuteContext(context, true);
    FreeContext(context);
  }
//---------------------------------------------------------------------------//
  MappedTempBuffer RenderCore::ReadbackBufferData(const GpuBuffer* aBuffer, uint64 anOffset, uint64 aByteSize)
  {
    ASSERT(anOffset + aByteSize <= aBuffer->GetByteSize());

    if (aBuffer->GetProperties().myCpuAccess == CpuMemoryAccessType::CPU_READ)
      LOG_WARNING("ReadbackBufferData() called with a CPU-readable buffer. Its better to directly map the buffer to avoid uneccessary temp-copies");

    GpuBufferResourceProperties props;
    props.myBufferProperties.myNumElements = MathUtil::Align(aByteSize - anOffset, Private_RenderCore::kReadbackBufferSizeIncrease); // Reserve a bit more size to make it more likely this buffer can be re-used for other, bigger readbacks
    props.myBufferProperties.myElementSizeBytes = 1u;
    props.myBufferProperties.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    props.myBufferProperties.myUsage = GpuBufferUsage::STAGING_READBACK;
    props.myIsShaderResource = false;
    props.myIsShaderWritable = false;
    TempBufferResource readbackBuffer  = AllocateTempBuffer(props, 0u, "Temp readback buffer");
    ASSERT(readbackBuffer.myBuffer != nullptr);

    CommandContext* ctx = AllocateContext(CommandListType::Graphics);
    ctx->CopyBufferRegion(readbackBuffer.myBuffer, 0u, aBuffer, anOffset, aByteSize);
    GetCommandQueue(CommandListType::Graphics)->ExecuteContext(ctx, true);
    FreeContext(ctx);

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
    props.myBufferProperties.myNumElements = MathUtil::Align(totalSize, Private_RenderCore::kReadbackBufferSizeIncrease); // Reserve a bit more size to make it more likely this buffer can be re-used for other, bigger readbacks
    props.myBufferProperties.myElementSizeBytes = 1u;
    props.myBufferProperties.myCpuAccess = CpuMemoryAccessType::CPU_READ;
    props.myBufferProperties.myUsage = GpuBufferUsage::STAGING_READBACK;
    props.myIsShaderResource = false;
    props.myIsShaderWritable = false;
    TempBufferResource readbackBuffer = AllocateTempBuffer(props, 0u, "Temp texture readback buffer");
    ASSERT(readbackBuffer.myBuffer != nullptr);

    const uint startSubresourceIndex = aTexture->GetSubresourceIndex(aStartSubLocation);

    CommandContext* ctx = AllocateContext(CommandListType::Graphics);
    for (uint subresource = 0; subresource < aNumSublocations; ++subresource)
    {
      TextureSubLocation subLocation = aTexture->GetSubresourceLocation(startSubresourceIndex + subresource);
      uint64 offset = subresourceOffsets[subresource];
      ctx->CopyTextureRegion(readbackBuffer.myBuffer, offset, aTexture, subLocation);
    }
    GetCommandQueue(CommandListType::Graphics)->ExecuteContext(ctx, true);
    FreeContext(ctx);

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
  CommandContext* RenderCore::AllocateContext(CommandListType aType)
  {
    ASSERT(aType == CommandListType::Graphics || aType == CommandListType::Compute,
      "CommandContext type % not implemented", (uint)aType);

    std::vector<std::unique_ptr<CommandContext>>& contextPool =
      aType == CommandListType::Graphics ? ourRenderContextPool : ourComputeContextPool;

    std::list<CommandContext*>& availableContextList =
      aType == CommandListType::Graphics ? ourAvailableRenderContexts : ourAvailableComputeContexts;

    if (!availableContextList.empty())
    {
      CommandContext* context = availableContextList.front();
      availableContextList.pop_front();
      return context;
    }

    contextPool.push_back(std::unique_ptr<CommandContext>(ourPlatformImpl->CreateContext(aType)));
    CommandContext* context = contextPool.back().get();
    
    return context;
  }
//---------------------------------------------------------------------------//
  void RenderCore::FreeContext(CommandContext* aContext)
  {
    CommandListType type = aContext->GetType();

    ASSERT(type == CommandListType::Graphics || type == CommandListType::Compute,
      "CommandContext type % not implemented", (uint)type);

    std::list<CommandContext*>& availableContextList =
      type == CommandListType::Graphics ? ourAvailableRenderContexts : ourAvailableComputeContexts;

    if (std::find(availableContextList.begin(), availableContextList.end(), aContext)
      != availableContextList.end())
      return;
    
    availableContextList.push_back(aContext);
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
  GpuQueryRange RenderCore::AllocateQueryRange(GpuQueryType aType, uint aNumQueries)
  {
    GpuQueryStorage& storage = ourQueryStorages[ourCurrQueryStorageIdx][(uint)aType];

    uint firstQueryIndex = 0u;
    const bool success = storage.AllocateQueries(aNumQueries, firstQueryIndex);
    ASSERT(success, "Not enough queries available for query type % (wants to allocate % queries, available are % queries)", (uint)aType, aNumQueries, storage.GetNumFreeQueries());

    return GpuQueryRange(storage.myQueryHeap.get(), firstQueryIndex, aNumQueries);
  }
//---------------------------------------------------------------------------//
  void RenderCore::FreeQueryRange(GpuQueryRange aQueryRange)
  {
    const uint queryType = (uint)aQueryRange.myHeap->myType;
    GpuQueryStorage& queryStorage = ourQueryStorages[ourCurrQueryStorageIdx][queryType];

    // Is this the last query-range that was allocated? Then deallocate the range in the storage again so it can be used again
    if (queryStorage.myNextFree == aQueryRange.myFirstQueryIndex + aQueryRange.myNumQueries)
      queryStorage.myNextFree = aQueryRange.myFirstQueryIndex + aQueryRange.myNumUsedQueries;

    ourUsedQueryRanges[queryType].push_back(glm::uvec2(aQueryRange.myFirstQueryIndex, aQueryRange.myFirstQueryIndex + aQueryRange.myNumUsedQueries));
  }
//---------------------------------------------------------------------------//
  bool RenderCore::IsFrameDone(uint64 aFrameIdx)
  {
    if (ourLastFrameDoneFences.IsEmpty() || ourLastFrameDoneFences[ourLastFrameDoneFences.Size() - 1].first < aFrameIdx)
      return false;

    if (ourLastFrameDoneFences[0].first > aFrameIdx)
      return true;

    CommandQueue* queue = GetCommandQueue(CommandListType::Graphics);
    for (uint i = 0u, e = ourLastFrameDoneFences.Size(); i < e; ++i)
      if (ourLastFrameDoneFences[i].first == aFrameIdx)
        return queue->IsFenceDone(ourLastFrameDoneFences[i].second);

    return false;
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
    GpuHazardData* hazardData = aResource->myHazardData.get();

    bool commandListNeedsWait[(uint)CommandListType::NUM] = { false, false, false };
    for (uint i = aSubresourceOffset, end = aSubresourceOffset + glm::min(aResource->GetNumSubresources() - aSubresourceOffset, aNumSubresources); i < end; ++i)
    {
      CommandListType cmdListType = hazardData->mySubresourceContexts[i];
      commandListNeedsWait[(uint)cmdListType] |= true;
    }

    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      if (commandListNeedsWait[i])
        WaitForIdle((CommandListType)i);
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  CommandQueue* RenderCore::GetCommandQueue(CommandListType aType)
  {
    return ourPlatformImpl->GetCommandQueue(aType);
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
        LOG_WARNING("Failed compiling shader %", program->GetDescription().myShaderFileName.c_str());
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



