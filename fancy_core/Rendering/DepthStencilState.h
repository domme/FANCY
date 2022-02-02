#pragma once

#include "Common/FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct DepthStencilFaceProperties
  {
    CompFunc  myStencilCompFunc = CompFunc::ALWAYS;
    StencilOp myStencilFailOp = StencilOp::KEEP;
    StencilOp myStencilDepthFailOp = StencilOp::KEEP;
    StencilOp myStencilPassOp = StencilOp::KEEP;
  };
//---------------------------------------------------------------------------//
  struct DepthStencilStateProperties
  {
    DepthStencilFaceProperties myFrontFace;
    DepthStencilFaceProperties myBackFace;

    bool      myDepthTestEnabled = true;
    bool      myDepthWriteEnabled = true;
    CompFunc  myDepthCompFunc = CompFunc::LESS;
    bool      myStencilEnabled = false;
    bool      myTwoSidedStencil = false;
    int       myStencilRef = 0;
    uint      myStencilReadMask = 0u;
    uint      myStencilWriteMask = 0u;
  };
//---------------------------------------------------------------------------//
  class DepthStencilState
  {
  public:
    DepthStencilState(DepthStencilStateProperties aProperties)
      : myProperties(aProperties)
    { }

    const DepthStencilStateProperties& GetProperties() const { return myProperties; }

  private:
    DepthStencilStateProperties myProperties;
  };
//---------------------------------------------------------------------------//
}