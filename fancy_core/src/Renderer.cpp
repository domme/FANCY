#include "Renderer.h"
#include "DepthStencilState.h"
#include "TextureRefs.h"
#include "GpuBuffer.h"
#include "GpuProgramCompiler.h"
#include "FileWatcher.h"
#include <mutex>
#include "PathService.h"
#include "GpuProgramPipeline.h"
#include <array>
#include "TextureLoader.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "GeometryData.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  FileWatcher* RenderCore::ourShaderFileWatcher = nullptr;

  std::map<uint64, SharedPtr<GpuProgram>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<GpuProgramPipeline>> RenderCore::ourGpuProgramPipelineCache;
  std::map<uint64, SharedPtr<Texture>> RenderCore::ourTextureCache;
  std::map<uint64, SharedPtr<Geometry::Mesh>> RenderCore::ourMeshCache;
  std::map<uint64, SharedPtr<Rendering::BlendState>> RenderCore::ourBlendStateCache;
  std::map<uint64, SharedPtr<Rendering::DepthStencilState>> RenderCore::ourDepthStencilStateCache;

  ScopedPtr<GpuProgramCompiler> RenderCore::ourShaderCompiler;
  std::shared_ptr<Texture> RenderCore::ourDefaultDiffuseTexture;
  std::shared_ptr<Texture> RenderCore::ourDefaultNormalTexture;
  std::shared_ptr<Texture> RenderCore::ourDefaultSpecularTexture;
//---------------------------------------------------------------------------//  
  void RenderCore::Init()
  {
    ourShaderFileWatcher = FANCY_NEW(FileWatcher, MemoryCategory::GENERAL);
    std::function<void(const String&)> onUpdatedFn(&RenderCore::OnShaderFileUpdated);
    std::function<void(const String&)> onDeletedFn(&RenderCore::OnShaderFileDeletedMoved);
    ourShaderFileWatcher->myOnFileUpdated.Connect(onUpdatedFn);
    ourShaderFileWatcher->myOnFileDeletedMoved.Connect(onDeletedFn);
    
    ourShaderCompiler = FANCY_NEW(GpuProgramCompiler, MemoryCategory::GENERAL);

    CreateDepthStencilState(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    CreateBlendState(BlendStateDesc::GetDefaultSolid());
    
    {
      ShaderVertexInputLayout& modelVertexLayout = ShaderVertexInputLayout::ourDefaultModelLayout;
      modelVertexLayout.clear();

      uint32 registerIndex = 0u;
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
  void RenderCore::PostInit()
  {
    ourDefaultDiffuseTexture.reset(FANCY_NEW(Texture, MemoryCategory::Textures));
    ourDefaultNormalTexture.reset(FANCY_NEW(Texture, MemoryCategory::Textures));
    ourDefaultSpecularTexture.reset(FANCY_NEW(Texture, MemoryCategory::Textures));

    {
      TextureParams params;
      params.myIsExternalTexture = false;
      params.eFormat = DataFormat::SRGB_8;
      params.u16Height = 1u;
      params.u16Width = 1u;
      params.myInternalRefIndex = (uint32) TextureRef::DEFAULT_DIFFUSE;

      TextureUploadData data(params);
      uint8 color[3] = { 0, 0, 0 };
      data.myData = color;

      ourDefaultDiffuseTexture->create(params, &data, 1);

      params.myInternalRefIndex = (uint32)TextureRef::DEFAULT_SPECULAR;
      ourDefaultSpecularTexture->create(params, &data, 1);
    }

    {
      TextureParams params;
      params.myIsExternalTexture = false;
      params.eFormat = DataFormat::RGB_8;
      params.u16Height = 1u;
      params.u16Width = 1u;
      params.myInternalRefIndex = (uint32)TextureRef::DEFAULT_NORMAL;

      TextureUploadData data(params);
      uint8 color[3] = { 128, 128, 128 };
      data.myData = color;
      
      ourDefaultNormalTexture->create(params, &data, 1);
    }

    ourTextureCache.insert(std::make_pair(ourDefaultDiffuseTexture->GetDescription().GetHash(), ourDefaultDiffuseTexture));
    ourTextureCache.insert(std::make_pair(ourDefaultSpecularTexture->GetDescription().GetHash(), ourDefaultSpecularTexture));
    ourTextureCache.insert(std::make_pair(ourDefaultNormalTexture->GetDescription().GetHash(), ourDefaultNormalTexture));
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    FANCY_DELETE(ourShaderFileWatcher, MemoryCategory::GENERAL);
    ourShaderFileWatcher = nullptr;

    FANCY_DELETE(ourShaderCompiler, MemoryCategory::GENERAL);
    ourShaderCompiler = nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgram> RenderCore::CreateGpuProgram(const GpuProgramDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourShaderCache.find(hash);
    if (it != ourShaderCache.end())
      return it->second;

    SharedPtr<GpuProgram> program(FANCY_NEW(GpuProgram, MemoryCategory::MATERIALS));
    if (program->SetFromDescription(aDesc, ourShaderCompiler))
    {
      ourShaderCache.insert(std::make_pair(hash, program));

      const String actualShaderPath =
        IO::PathService::convertToAbsPath(ourShaderCompiler->ResolvePlatformShaderPath(aDesc.myShaderFileName));
      
      ourShaderFileWatcher->AddFileWatch(actualShaderPath);

      return program;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourGpuProgramPipelineCache.find(hash);
    if (it != ourGpuProgramPipelineCache.end())
      return it->second;

    std::array<SharedPtr<GpuProgram>, (uint32)ShaderStage::NUM> pipelinePrograms {nullptr};
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      if (!aDesc.myGpuPrograms[i].myShaderFileName.empty())
        pipelinePrograms[i] = CreateGpuProgram(aDesc.myGpuPrograms[i]);
    }

    SharedPtr<GpuProgramPipeline> pipeline(FANCY_NEW(GpuProgramPipeline, MemoryCategory::MATERIALS));
    pipeline->SetFromShaders(pipelinePrograms);

    ourGpuProgramPipelineCache.insert(std::make_pair(hash, pipeline));

    return pipeline;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::GetTexture(uint64 aDescHash)
  {
    auto it = ourTextureCache.find(aDescHash);
    if (it != ourTextureCache.end())
      return it->second;

    return nullptr;
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
  SharedPtr<Rendering::BlendState> RenderCore::CreateBlendState(const Rendering::BlendStateDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = ourBlendStateCache.find(aDesc.GetHash());
    if (it != ourBlendStateCache.end())
      return it->second;

    SharedPtr<Rendering::BlendState> blendState(FANCY_NEW(Rendering::BlendState, MemoryCategory::GENERAL));
    blendState->SetFromDescription(aDesc);
    
    ourBlendStateCache.insert(std::make_pair(aDesc.GetHash(), blendState));
    return blendState;
  }
  //---------------------------------------------------------------------------//
  SharedPtr<Rendering::DepthStencilState> RenderCore::CreateDepthStencilState(const Rendering::DepthStencilStateDesc& aDesc)
  {
    if (aDesc.IsEmpty())
      return nullptr;

    auto it = ourDepthStencilStateCache.find(aDesc.GetHash());
    if (it != ourDepthStencilStateCache.end())
      return it->second;

    SharedPtr<Rendering::DepthStencilState> depthStencilState(FANCY_NEW(Rendering::DepthStencilState, MemoryCategory::GENERAL));
    depthStencilState->SetFromDescription(aDesc);

    ourDepthStencilStateCache.insert(std::make_pair(aDesc.GetHash(), depthStencilState));
    return depthStencilState;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> RenderCore::GetMesh(uint64 aVertexIndexHash)
  {
    auto it = ourMeshCache.find(aVertexIndexHash);
    if (it != ourMeshCache.end())
      return it->second;

    SharedPtr<Geometry::Mesh> mesh(FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY));
    if (IO::BinaryCache::read(mesh, aVertexIndexHash, 0u))
    {
      ourMeshCache.insert(std::make_pair(aVertexIndexHash, mesh));
      return mesh;
    }

    return nullptr;
  }
  //---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> RenderCore::CreateMesh(const Geometry::MeshDesc& aDesc,
    const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
    const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices)
  {
    SharedPtr<Geometry::Mesh> mesh = GetMesh(aDesc.myVertexAndIndexHash);
    if (mesh != nullptr)
      return mesh;

    const uint numSubMeshes = aDesc.myVertexLayouts.size();
    ASSERT(numSubMeshes == someVertexDatas.size() && numSubMeshes == someIndexDatas.size());

    Geometry::GeometryDataList vGeometryDatas;

    for (uint iSubMesh = 0u; iSubMesh < numSubMeshes; ++iSubMesh)
    {
      Geometry::GeometryData* pGeometryData = FANCY_NEW(Geometry::GeometryData, MemoryCategory::GEOMETRY);

      const Rendering::GeometryVertexLayout& vertexLayout = aDesc.myVertexLayouts[iSubMesh];

      // Construct the vertex buffer
      void* ptrToVertexData = someVertexDatas[iSubMesh];
      uint numVertices = someNumVertices[iSubMesh];

      Rendering::GpuBuffer* vertexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.bIsMultiBuffered = false;
      bufferParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::VERTEX_BUFFER);
      bufferParams.uAccessFlags = static_cast<uint>(Rendering::GpuResourceAccessFlags::NONE);
      bufferParams.uNumElements = numVertices;
      bufferParams.uElementSizeBytes = vertexLayout.getStrideBytes();

      vertexBuffer->create(bufferParams, ptrToVertexData);
      pGeometryData->setVertexLayout(vertexLayout);
      pGeometryData->setVertexBuffer(vertexBuffer);

      // Construct the index buffer
      void* ptrToIndexData = someIndexDatas[iSubMesh];
      uint numIndices = someNumIndices[iSubMesh];

      Rendering::GpuBuffer* indexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams indexBufParams;
      indexBufParams.bIsMultiBuffered = false;
      indexBufParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::INDEX_BUFFER);
      indexBufParams.uAccessFlags = static_cast<uint32>(Rendering::GpuResourceAccessFlags::NONE);
      indexBufParams.uNumElements = numIndices;
      indexBufParams.uElementSizeBytes = sizeof(uint32);

      indexBuffer->create(indexBufParams, ptrToIndexData);
      pGeometryData->setIndexBuffer(indexBuffer);

      vGeometryDatas.push_back(pGeometryData);
    }

    mesh = SharedPtr<Geometry::Mesh>(FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY));
    mesh->setGeometryDataList(vGeometryDatas);
    mesh->SetVertexIndexHash(aDesc.myVertexAndIndexHash);

    ourMeshCache.insert(std::make_pair(aDesc.myVertexAndIndexHash, mesh));
    IO::BinaryCache::write(mesh, someVertexDatas, someIndexDatas);

    return mesh;
  } 
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureDesc& aTextureDesc)
  {
    if (aTextureDesc.IsEmpty())
      return nullptr;

    TextureDesc desc = aTextureDesc;

    if (aTextureDesc.myIsExternalTexture)
    {
      String texPathAbs = aTextureDesc.mySourcePath;
      String texPathRel = aTextureDesc.mySourcePath;
      if (!IO::PathService::isAbsolutePath(texPathAbs))
        IO::PathService::convertToAbsPath(texPathAbs);
      else
        texPathRel = IO::PathService::toRelPath(texPathAbs);
      
      desc.mySourcePath = texPathAbs;
    }

    auto it = ourTextureCache.find(desc.GetHash());
    if (it != ourTextureCache.end())
      return it->second;

    ASSERT(aTextureDesc.myIsExternalTexture, 
      "Couldn't find internal texture with refIndex %", 
        aTextureDesc.myInternalRefIndex);

    return CreateTexture(desc.mySourcePath);
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const String& aTexturePath)
  {
    String texPathAbs = aTexturePath;
    String texPathRel = aTexturePath;
    if (!IO::PathService::isAbsolutePath(aTexturePath))
      IO::PathService::convertToAbsPath(texPathAbs);
    else
      texPathRel = IO::PathService::toRelPath(texPathAbs);

    TextureDesc desc;
    desc.mySourcePath = texPathAbs;
    desc.myIsExternalTexture = true;
    uint64 hash = desc.GetHash();

    auto it = ourTextureCache.find(hash);
    if (it != ourTextureCache.end())
      return it->second;

    std::vector<uint8> textureBytes;
    IO::TextureLoadInfo textureInfo;
    if (!IO::TextureLoader::loadTexture(texPathAbs, textureBytes, textureInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return nullptr;
    }

    if (textureInfo.bitsPerPixel / textureInfo.numChannels != 8u)
    {
      LOG_ERROR("Unsupported texture format: %", texPathAbs);
      return nullptr;
    }

    SharedPtr<Texture> texture(FANCY_NEW(Texture, MemoryCategory::TEXTURES));

    TextureParams texParams;
    texParams.myIsExternalTexture = true;
    texParams.path = texPathRel;
    texParams.bIsDepthStencil = false;
    texParams.eFormat = textureInfo.numChannels == 3u ? DataFormat::SRGB_8 : DataFormat::SRGB_8_A_8;
    texParams.u16Width = textureInfo.width;
    texParams.u16Height = textureInfo.height;
    texParams.u16Depth = 0u;
    texParams.uAccessFlags = (uint32)GpuResourceAccessFlags::NONE;

    TextureUploadData uploadData;
    uploadData.myData = &textureBytes[0];
    uploadData.myPixelSizeBytes = textureInfo.bitsPerPixel / 8u;
    uploadData.myRowSizeBytes = textureInfo.width * uploadData.myPixelSizeBytes;
    uploadData.mySliceSizeBytes = textureInfo.width * textureInfo.height * uploadData.myPixelSizeBytes;
    uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

    texture->create(texParams, &uploadData, 1u);

    if (!texture->isValid())
    {
      LOG_ERROR("Failed to upload pixel data of texture %", texPathAbs);
      return nullptr;
    }

    IO::BinaryCache::write(texture, uploadData);
    ourTextureCache.insert(std::make_pair(hash, texture));

    return texture;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas, uint32 aNumUploadDatas)
  {
    SharedPtr<Texture> texture(new Texture);
    texture->create(someParams, someUploadDatas, aNumUploadDatas);

    return texture->isValid() ? texture : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBuffer> RenderCore::CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData /* = nullptr */)
  {
    SharedPtr<GpuBuffer> buffer(new GpuBuffer);
    buffer->create(someParams, someInitialData);
    return buffer->isValid() ? buffer : nullptr;
  }
//---------------------------------------------------------------------------//
  void RenderCore::UpdateBufferData(GpuBuffer* aBuffer, void* aData, uint32 aDataSizeBytes, uint32 aByteOffsetFromBuffer /* = 0 */)
  {
    ASSERT(aByteOffsetFromBuffer + aDataSizeBytes <= aBuffer->GetSizeBytes());

    const GpuBufferCreationParams& bufParams = aBuffer->GetParameters();

    if (bufParams.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE)
    {
      uint8* dest = static_cast<uint8*>(aBuffer->Lock(GpuResoruceLockOption::WRITE));
      ASSERT(dest != nullptr);
      memcpy(dest + aByteOffsetFromBuffer, aData, aDataSizeBytes);
      aBuffer->Unlock();
    }
    else
    {
      RenderContext::UpdateBufferData(aBuffer, aData, aByteOffsetFromBuffer, aDataSizeBytes);
    }
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
        IO::PathService::convertToAbsPath(ourShaderCompiler->ResolvePlatformShaderPath(desc.myShaderFileName));

      if (actualShaderPath == aShaderFile)
        programsToRecompile.push_back(program);
    }

    for (GpuProgram* program : programsToRecompile)
      program->SetFromDescription(program->GetDescription(), ourShaderCompiler);


    // Check which pipelines need to be updated...
    std::vector<GpuProgramPipeline*> changedPipelines;
    for (auto it = ourGpuProgramPipelineCache.begin(); it != ourGpuProgramPipelineCache.end(); ++it)
    {
      GpuProgramPipeline* pipeline = it->second.get();

      for (GpuProgram* changedProgram : programsToRecompile)
      {
        const uint stage = static_cast<uint>(changedProgram->getShaderStage());
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
} }

