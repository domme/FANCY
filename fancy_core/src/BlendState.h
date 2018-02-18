#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct BlendStateDesc
  {
    static BlendStateDesc GetDefaultSolid();

    BlendStateDesc();

    bool operator==(const BlendStateDesc& anOther) const;
    uint64 GetHash() const;

    bool myAlphaToCoverageEnabled;
    bool myBlendStatePerRT;
    bool myAlphaSeparateBlend[Constants::kMaxNumRenderTargets];
    bool myBlendEnabled[Constants::kMaxNumRenderTargets];
    uint mySrcBlend[Constants::kMaxNumRenderTargets];
    uint myDestBlend[Constants::kMaxNumRenderTargets];
    uint myBlendOp[Constants::kMaxNumRenderTargets];
    uint mySrcBlendAlpha[Constants::kMaxNumRenderTargets];
    uint myDestBlendAlpha[Constants::kMaxNumRenderTargets];
    uint myBlendOpAlpha[Constants::kMaxNumRenderTargets];
    uint myRTwriteMask[Constants::kMaxNumRenderTargets];
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
  bool        myAlphaSeparateBlend[Constants::kMaxNumRenderTargets];
  bool        myBlendEnabled[Constants::kMaxNumRenderTargets];
  BlendInput  mySrcBlend[Constants::kMaxNumRenderTargets];
  BlendInput  myDestBlend[Constants::kMaxNumRenderTargets];
  BlendOp     myBlendOp[Constants::kMaxNumRenderTargets];
  BlendInput  mySrcBlendAlpha[Constants::kMaxNumRenderTargets];
  BlendInput  myDestBlendAlpha[Constants::kMaxNumRenderTargets];
  BlendOp     myBlendOpAlpha[Constants::kMaxNumRenderTargets];
  uint        myRTwriteMask[Constants::kMaxNumRenderTargets];
//---------------------------------------------------------------------------//
};

} }  // end of namespace Fancy::Rendering
