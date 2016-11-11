#include "Renderer.h"
#include "DepthStencilState.h"
#include "TextureRefs.h"
#include "GpuBuffer.h"
#include "GpuProgramCompiler.h"
#include "FileWatcher.h"
#include <mutex>
#include "PathService.h"

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  FileWatcher* RenderCore::ourShaderFileWatcher = nullptr;

  std::map<uint64, SharedPtr<GpuProgram>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<GpuProgramPipeline>> RenderCore::ourGpuProgramPipelineCache;

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

    DepthStencilState defaultDepthStencilState;
    defaultDepthStencilState.SetFromDescription(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    DepthStencilState::Register(defaultDepthStencilState);

    BlendState defaultBlendstate;
    defaultBlendstate.SetFromDescription(BlendStateDesc::GetDefaultSolid());
    BlendState::Register(defaultBlendstate);

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

    Texture::Register(ourDefaultDiffuseTexture.get());
    Texture::Register(ourDefaultNormalTexture.get());
    Texture::Register(ourDefaultSpecularTexture.get());
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

    GpuProgramPipeline::NotifyChangedShaders(programsToRecompile);
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileDeletedMoved(const String& aShaderFile)
  {
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc)
  {

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
} }

