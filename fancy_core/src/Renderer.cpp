#include "Renderer.h"
#include "DepthStencilState.h"
#include "ShaderConstantsManager.h"
#include "ResourceBinding.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  std::shared_ptr<Texture> RenderCore::ourDefaultDiffuseTexture;
  std::shared_ptr<Texture> RenderCore::ourDefaultNormalTexture;
  std::shared_ptr<Texture> RenderCore::ourDefaultSpecularTexture;
//---------------------------------------------------------------------------//  
  void RenderCore::Init()
  {
    DepthStencilState defaultDepthStencilState;
    defaultDepthStencilState.SetFromDescription(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    DepthStencilState::Register(defaultDepthStencilState);

    BlendState defaultBlendstate;
    defaultBlendstate.SetFromDescription(BlendStateDesc::GetDefaultSolid());
    BlendState::Register(defaultBlendstate);

    ShaderConstantsManager::Init();

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

      TextureUploadData data(params);
      uint8 color[3] = { 0, 0, 0 };
      data.myData = color;

      ourDefaultDiffuseTexture->create(params, &data, 1);
      ourDefaultSpecularTexture->create(params, &data, 1);
    }

    {
      TextureParams params;
      params.myIsExternalTexture = false;
      params.eFormat = DataFormat::RGB_8;
      params.u16Height = 1u;
      params.u16Width = 1u;

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
    ShaderConstantsManager::Shutdown();
  }
//---------------------------------------------------------------------------//
} }

