#pragma once

// TODO: Fix this mirror-madness

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct BlendStateDesc
  {
    static BlendStateDesc GetDefaultSolid();

    BlendStateDesc();

    bool operator==(const BlendStateDesc& anOther) const;
    uint64 GetHash() const;

    bool myAlphaToCoverageEnabled;
    bool myBlendStatePerRT;
    bool myAlphaSeparateBlend[RenderConstants::kMaxNumRenderTargets];
    bool myBlendEnabled[RenderConstants::kMaxNumRenderTargets];
    uint mySrcBlend[RenderConstants::kMaxNumRenderTargets];
    uint myDestBlend[RenderConstants::kMaxNumRenderTargets];
    uint myBlendOp[RenderConstants::kMaxNumRenderTargets];
    uint mySrcBlendAlpha[RenderConstants::kMaxNumRenderTargets];
    uint myDestBlendAlpha[RenderConstants::kMaxNumRenderTargets];
    uint myBlendOpAlpha[RenderConstants::kMaxNumRenderTargets];
    uint myRTwriteMask[RenderConstants::kMaxNumRenderTargets];
  };
//---------------------------------------------------------------------------//
class BlendState {

public:
  BlendState();

  bool operator==(const BlendState& clOther) const;
  bool operator==(const BlendStateDesc& clOther) const;

  BlendStateDesc GetDescription() const;
  void SetFromDescription(const BlendStateDesc& aDesc);

  uint64 GetHash() const;
  
  bool        myAlphaToCoverageEnabled;
  bool        myBlendStatePerRT;
  bool        myAlphaSeparateBlend[RenderConstants::kMaxNumRenderTargets];
  bool        myBlendEnabled[RenderConstants::kMaxNumRenderTargets];
  BlendFactor  mySrcBlend[RenderConstants::kMaxNumRenderTargets];
  BlendFactor  myDestBlend[RenderConstants::kMaxNumRenderTargets];
  BlendOp     myBlendOp[RenderConstants::kMaxNumRenderTargets];
  BlendFactor  mySrcBlendAlpha[RenderConstants::kMaxNumRenderTargets];
  BlendFactor  myDestBlendAlpha[RenderConstants::kMaxNumRenderTargets];
  BlendOp     myBlendOpAlpha[RenderConstants::kMaxNumRenderTargets];
  uint        myRTwriteMask[RenderConstants::kMaxNumRenderTargets];
//---------------------------------------------------------------------------//
};

}
