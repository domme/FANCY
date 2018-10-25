#include "RenderCore.h"

#include <mutex>
#include <array>

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
#include "RenderingStartupParameters.h"
#include "MeshData.h"
#include "CommandContextDX12.h"
#include "TextureViewProperties.h"

#include <xxHash/xxhash.h>

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
  }
//---------------------------------------------------------------------------//
  std::unique_ptr<RenderCore_Platform> RenderCore::ourPlatformImpl;

  std::map<uint64, SharedPtr<GpuProgram>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<GpuProgramPipeline>> RenderCore::ourGpuProgramPipelineCache;
  std::map<uint64, SharedPtr<BlendState>> RenderCore::ourBlendStateCache;
  std::map<uint64, SharedPtr<DepthStencilState>> RenderCore::ourDepthStencilStateCache;

  std::vector<std::unique_ptr<CommandContext>> RenderCore::ourRenderContextPool;
  std::vector<std::unique_ptr<CommandContext>> RenderCore::ourComputeContextPool;
  std::list<CommandContext*> RenderCore::ourAvailableRenderContexts;
  std::list<CommandContext*> RenderCore::ourAvailableComputeContexts;
  
  SharedPtr<Texture> RenderCore::ourDefaultDiffuseTexture;
  SharedPtr<Texture> RenderCore::ourDefaultNormalTexture;
  SharedPtr<Texture> RenderCore::ourDefaultSpecularTexture;
  SharedPtr<DepthStencilState> RenderCore::ourDefaultDepthStencilState;
  SharedPtr<BlendState> RenderCore::ourDefaultBlendState;
  SharedPtr<GpuProgram> RenderCore::ourComputeMipMapShader;
  
  std::unique_ptr<FileWatcher> RenderCore::ourShaderFileWatcher;
  std::unique_ptr<GpuProgramCompiler> RenderCore::ourShaderCompiler;

  std::vector<std::unique_ptr<GpuRingBuffer>> RenderCore::ourRingBufferPool;
  std::list<GpuRingBuffer*> RenderCore::ourAvailableRingBuffers;
  std::list<std::pair<uint64, GpuRingBuffer*>> RenderCore::ourUsedRingBuffers;
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
  void RenderCore::EndFrame()
  {

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
    params.myCpuAccess = GpuMemoryAccessType::CPU_WRITE;
    buf->Create(params, GpuResoruceLockOption::WRITE, aName);
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
      props.eFormat = DataFormat::SRGB_8;
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
      props.eFormat = DataFormat::RGB_8;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myHeight = 1u;
      props.myWidth = 1u;
      props.path = TextureRef::ToString(TextureRef::DEFAULT_NORMAL);

      TextureSubData data(props);
      uint8 color[3] = { 128, 128, 128 };
      data.myData = color;

      ourDefaultNormalTexture = CreateTexture(props, "Default_Normal", &data, 1);
    }

    {
      GpuProgramDesc desc;
      desc.myMainFunction = "main";
      desc.myShaderStage = (uint) ShaderStage::COMPUTE;
      desc.myShaderFileName = "ComputeMipmapCS";
      ourComputeMipMapShader = CreateGpuProgram(desc);
      ASSERT(ourComputeMipMapShader != nullptr);
    }
    
    ourDefaultDepthStencilState = CreateDepthStencilState(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    ASSERT(ourDefaultDepthStencilState != nullptr);

    ourDefaultBlendState = CreateBlendState(BlendStateDesc::GetDefaultSolid());
    ASSERT(ourDefaultBlendState != nullptr);
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
    ourComputeMipMapShader.reset();

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
  DataFormat RenderCore::ResolveFormat(DataFormat aFormat)
  {
    return ourPlatformImpl->ResolveFormat(aFormat);
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
      bufferParams.myCpuAccess = GpuMemoryAccessType::NO_CPU_ACCESS;
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
      indexBufParams.myCpuAccess = GpuMemoryAccessType::NO_CPU_ACCESS;
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
    DataFormat format = RenderCore::ResolveFormat(someProperties.myFormat);
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
  void RenderCore::UpdateBufferData(GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize)
  {
    ASSERT(aDestOffset + aByteSize <= aDestBuffer->GetByteSize());

    const GpuBufferProperties& bufParams = aDestBuffer->GetProperties();

    if (bufParams.myCpuAccess == GpuMemoryAccessType::CPU_WRITE)
    {
      uint8* dest = static_cast<uint8*>(aDestBuffer->Lock(GpuResoruceLockOption::WRITE));
      ASSERT(dest != nullptr);
      memcpy(dest + aDestOffset, aDataPtr, aByteSize);
      aDestBuffer->Unlock();
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
  void RenderCore::ComputeMipMaps(const SharedPtr<Texture>& aDestTexture)
  {
    const TextureProperties& destTexProps = aDestTexture->GetProperties();
    if(destTexProps.myNumMipLevels == 1)
      return;

    const DataFormatInfo& destTexFormatInfo = DataFormatInfo::GetFormatInfo(destTexProps.eFormat);

    // TODO: Check if the creation of a temp-texture is neccessary (aDestTexture is already shaderWritable)
    TextureProperties tempProps = aDestTexture->GetProperties();
    tempProps.myNumMipLevels = destTexProps.myNumMipLevels - 1;
    tempProps.myIsShaderWritable = true;
    tempProps.myWidth = (uint) (destTexProps.myWidth / 2.0f);
    tempProps.myHeight = (uint) (destTexProps.myHeight / 2.0f);
    if (tempProps.myWidth == 0u && tempProps.myHeight == 0u)
      return;
    tempProps.myWidth = glm::max(1u, tempProps.myWidth);
    tempProps.myHeight = glm::max(1u, tempProps.myHeight);

    SharedPtr<Texture> tempTexPtr = CreateTexture(tempProps);
    Texture* tempTex = tempTexPtr.get();
    ASSERT(tempTex != nullptr);

    TextureViewProperties readProps;
    readProps.myNumMipLevels = 1;
    readProps.myFormat = aDestTexture->GetProperties().eFormat;
    readProps.myDimension = GpuResourceDimension::TEXTURE_2D;

    TextureViewProperties writeProps = readProps;
    writeProps.myFormat = DataFormatInfo::GetNonSRGBformat(readProps.myFormat);
    writeProps.myIsShaderWritable = true;

    const uint numMips = destTexProps.myNumMipLevels;
    const uint kMaxNumMips = 17u;
    ASSERT(numMips <= kMaxNumMips);
    FixedArray<SharedPtr<TextureView>, kMaxNumMips> readViews;
    FixedArray<SharedPtr<TextureView>, kMaxNumMips> writeViews;

    readProps.myMipIndex = 0u;
    readViews[0] = CreateTextureView(aDestTexture, readProps);
    ASSERT(readViews[0] != nullptr);

    for (uint mip = 1u; mip < numMips; ++mip)
    {
      uint tmpTexMip = mip-1u;
      readProps.myMipIndex = tmpTexMip;
      writeProps.myMipIndex = tmpTexMip;
      readViews[mip] = CreateTextureView(tempTexPtr, readProps);
      writeViews[mip] = CreateTextureView(tempTexPtr, writeProps);
      ASSERT(readViews[mip] != nullptr && writeViews[mip] != nullptr);
    }
    
    CommandQueue* queue = GetCommandQueue(CommandListType::Graphics);
    CommandContext* ctx = AllocateContext(CommandListType::Graphics);

    ctx->SetComputeProgram(ourComputeMipMapShader.get());

    glm::uvec2 size(tempProps.myWidth, tempProps.myHeight);
    uint mip = 1u;
    for (; mip < numMips && size.x > 0 && size.y > 0; ++mip)
    {
      struct CBuffer
      {
        glm::float2 mySizeOnMipInv;
        int myMip;
        int myIsSRGB;
      };
      CBuffer cBuffer = 
      {
        glm::float2(1.0f / size.x, 1.0f / size.y),
        (int) mip,
        destTexFormatInfo.mySRGB ? 1 : 0
      };
      ctx->BindConstantBuffer(&cBuffer, sizeof(cBuffer), 0u);

      const GpuResourceView* resources[] = { readViews[mip-1].get(), writeViews[mip].get()};
      ctx->BindResourceSet(resources, 2, 1u);

      ctx->Dispatch(size.x, size.y, 1);
     
      size /= 2u;
    }

    ctx->TransitionResource(
      tempTex, GpuResourceTransition::TO_COPY_SRC, 
      aDestTexture.get(), GpuResourceTransition::TO_COPY_DEST);

    for (uint i = 1u; i < mip; ++i)
      ctx->CopyTextureRegion(aDestTexture.get(), TextureSubLocation(i), glm::uvec3(0u), tempTex, TextureSubLocation(i-1));

    queue->ExecuteContext(ctx, true);
    FreeContext(ctx);
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
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileDeletedMoved(const String& aShaderFile)
  {
  }
//---------------------------------------------------------------------------//
}



