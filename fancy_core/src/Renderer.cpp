#include "Renderer.h"
#include "DepthStencilState.h"
#include "ShaderConstantsManager.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  void RenderingSubsystem::Init()
  {
    // Register common depth-stencil and blend-states
    //---------------------------------------------------------------------------//
    DepthStencilState defaultDepthStencilState;
    defaultDepthStencilState.SetFromDescription(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    DepthStencilState::Register(defaultDepthStencilState);

    BlendState defaultBlendstate;
    defaultBlendstate.SetFromDescription(BlendStateDesc::GetDefaultSolid());
    BlendState::Register(defaultBlendstate);

    ShaderConstantsManager::Init();
    //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystem::Shutdown()
  {
    ShaderConstantsManager::Shutdown();
  }
//---------------------------------------------------------------------------//
} }

