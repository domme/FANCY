#include "Renderer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  void RenderingSubsystem::Init()
  {
    // Init default depthStencil states
    {
      DepthStencilState depthStencilState(_N(DepthStencilState_DefaultDepthState));
      depthStencilState.myStencilEnabled = false;
      depthStencilState.myDepthTestEnabled = true;
      depthStencilState.myDepthWriteEnabled = true;
      depthStencilState.myDepthCompFunc = Rendering::CompFunc::LESS;
      DepthStencilState::registerWithName(depthStencilState);
    }
    // More to come here...
    
    // Init default blendStates
    {
      BlendState blendState(_N(BlendState_Solid));
      blendState.setBlendStatePerRT(false);
      blendState.setBlendEnabled(0u, false);
      blendState.setRTwriteMask(0u, UINT_MAX);
      BlendState::registerWithName(blendState);
    }
    // More to come here...
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystem::Shutdown()
  {
    
  }
//---------------------------------------------------------------------------//
} }

