#include "Renderer.h"
#include "DepthStencilState.h"
#include "ShaderConstantsManager.h"
#include "ResourceBinding.h"

namespace Fancy { namespace Rendering {
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

    ResourceBinding::Register();

  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    ShaderConstantsManager::Shutdown();
  }
//---------------------------------------------------------------------------//
} }

