#include "Renderer.h"
#include "DepthStencilState.h"

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
    //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystem::Shutdown()
  {
    
  }
//---------------------------------------------------------------------------//
} }

