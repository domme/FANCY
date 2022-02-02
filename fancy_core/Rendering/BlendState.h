#pragma once

#include "Common/FancyCoreDefines.h"
#include "RenderEnums.h"
#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct BlendStateRenderTargetProperties
  {
    bool myAlphaSeparateBlend = false;
    bool myBlendEnabled = false;
    BlendFactor mySrcBlendFactor = BlendFactor::ONE;
    BlendFactor mySrcBlendAlphaFactor = BlendFactor::ONE;
    BlendFactor myDstBlendFactor = BlendFactor::ZERO;
    BlendFactor myDstBlendAlphaFactor = BlendFactor::ZERO;
    BlendOp myBlendOp = BlendOp::ADD;
    BlendOp myBlendOpAlpha = BlendOp::ADD;
    uint myColorChannelWriteMask = UINT_MAX;
  };
//---------------------------------------------------------------------------//
  struct BlendStateProperties
  {
    BlendStateRenderTargetProperties myRendertargetProperties[RenderConstants::kMaxNumRenderTargets];
    bool myAlphaToCoverageEnabled = false;
    bool myBlendStatePerRT = false;
    bool myLogicOpEnabled = false;
    LogicOp myLogicOp = LogicOp::NO_OP;
  };
//---------------------------------------------------------------------------//
  class BlendState 
  {
  public:
    explicit BlendState(BlendStateProperties aProperties) 
      : myProperties(aProperties) 
    {}

    const BlendStateProperties& GetProperties() const { return myProperties; }
  private:
    BlendStateProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
