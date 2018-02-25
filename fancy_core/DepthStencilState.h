#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct DepthStencilStateDesc
  {
    static DepthStencilStateDesc GetDefaultDepthNoStencil();

    DepthStencilStateDesc();
    bool operator==(const DepthStencilStateDesc& anOther) const;
    uint64 GetHash() const;

    bool myDepthTestEnabled;
    bool myDepthWriteEnabled;
    uint myDepthCompFunc;
    bool myStencilEnabled;
    bool myTwoSidedStencil;
    int myStencilRef;
    uint myStencilReadMask;
    uint myStencilCompFunc[(uint)FaceType::NUM];
    uint myStencilWriteMask[(uint)FaceType::NUM];
    uint myStencilFailOp[(uint)FaceType::NUM];
    uint myStencilDepthFailOp[(uint)FaceType::NUM];
    uint myStencilPassOp[(uint)FaceType::NUM];
  };
//---------------------------------------------------------------------------//
  class DepthStencilState
  {
  public:
      DepthStencilState();
      bool operator==(const DepthStencilState& clOther) const;
      bool operator==(const DepthStencilStateDesc& aDesc) const;

      DepthStencilStateDesc GetDescription() const;
      void SetFromDescription(const DepthStencilStateDesc& aDesc);
      uint64 GetHash() const;

      bool      myDepthTestEnabled;
      bool      myDepthWriteEnabled;
      CompFunc  myDepthCompFunc;
      bool      myStencilEnabled;
      bool      myTwoSidedStencil;
      int       myStencilRef;
      uint      myStencilReadMask;
      CompFunc  myStencilCompFunc[(uint)FaceType::NUM];
      uint      myStencilWriteMask[(uint)FaceType::NUM];
      StencilOp myStencilFailOp[(uint)FaceType::NUM];
      StencilOp myStencilDepthFailOp[(uint)FaceType::NUM];
      StencilOp myStencilPassOp[(uint)FaceType::NUM];
  };
//---------------------------------------------------------------------------//
}