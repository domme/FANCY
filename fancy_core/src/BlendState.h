#pragma once

#include "ObjectName.h"
#include "RendererPrerequisites.h"
#include "Serializable.h"

namespace Fancy{namespace IO{
  class Serializer;
}}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct BlendStateDesc : public DescriptionBase
  {
    static BlendStateDesc GetDefaultSolid();

    BlendStateDesc();
    ~BlendStateDesc() override {}

    bool IsEmpty() const override { return false; }

    bool operator==(const BlendStateDesc& anOther) const;
    uint64 GetHash() const override;

    ObjectName GetTypeName() const override { return _N(BlendState); }
    void Serialize(IO::Serializer* aSerializer) override;

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
  SERIALIZABLE_RESOURCE(BlendState)

  explicit BlendState();
  ~BlendState() {}
  friend class RenderCoreDX12;

  bool operator==(const BlendState& clOther) const;
  bool operator==(const BlendStateDesc& clOther) const;

  BlendStateDesc GetDescription() const;
  void SetFromDescription(const BlendStateDesc& aDesc);

  uint64 GetHash() const;
    
  bool getAlphaToCoverageEnabled() const { return myAlphaToCoverageEnabled; }
  void setAlphaToCoverageEnabled(bool val) { myIsDirty |= myAlphaToCoverageEnabled != val; myAlphaToCoverageEnabled = val; }

  bool getBlendStatePerRT() const { return myBlendStatePerRT; }
  void setBlendStatePerRT(bool val) { myIsDirty |= myBlendStatePerRT != val; myBlendStatePerRT = val; }
  
  bool getAlphaSeparateBlendEnabled(uint _u32rtIndex) const {return myAlphaSeparateBlend[_u32rtIndex]; }
  void setAlphaSeparateBlendEnabled(uint _u32rtIndex, bool val) { myIsDirty |= myAlphaSeparateBlend[_u32rtIndex] != val;  myAlphaSeparateBlend[_u32rtIndex] = val; }

  bool getBlendEnabled(uint _u32rtIndex) const {return myBlendEnabled[_u32rtIndex]; }
  void setBlendEnabled(uint _u32rtIndex, bool val) { myIsDirty |= myBlendEnabled[_u32rtIndex] != val;  myBlendEnabled[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlend(uint _u32rtIndex) const { return mySrcBlend[_u32rtIndex]; }
  void setSrcBlend(uint _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= mySrcBlend[_u32rtIndex] != val;  mySrcBlend[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getDestBlend(uint _u32rtIndex) const { return myDestBlend[_u32rtIndex]; }
  void setDestBlend(uint _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= myDestBlend[_u32rtIndex] != val;  myDestBlend[_u32rtIndex] = val; }

  Fancy::Rendering::BlendOp getBlendOp(uint _u32rtIndex) const { return myBlendOp[_u32rtIndex]; }
  void setBlendOp(uint _u32rtIndex, Fancy::Rendering::BlendOp val) { myIsDirty |= myBlendOp[_u32rtIndex] != val;  myBlendOp[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlendAlpha(uint _u32rtIndex) const { return mySrcBlendAlpha[_u32rtIndex]; }
  void setSrcBlendAlpha(uint _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= mySrcBlendAlpha[_u32rtIndex] != val; mySrcBlendAlpha[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getDestBlendAlpha(uint _u32rtIndex) const { return myDestBlendAlpha[_u32rtIndex]; }
  void setDestBlendAlpha(uint _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= myDestBlendAlpha[_u32rtIndex] != val;  myDestBlendAlpha[_u32rtIndex] = val; }

  Fancy::Rendering::BlendOp getBlendOpAlpha(uint _u32rtIndex) const { return myBlendOpAlpha[_u32rtIndex]; }
  void setBlendOpAlpha(uint _u32rtIndex, Fancy::Rendering::BlendOp val) { myIsDirty |= myBlendOpAlpha[_u32rtIndex] != val; myBlendOpAlpha[_u32rtIndex] = val; }

  Fancy::uint getRTwriteMask(uint _u32rtIndex) const { return myRTwriteMask[_u32rtIndex]; }
  void setRTwriteMask(uint _u32rtIndex, Fancy::uint val) { myIsDirty |= myRTwriteMask[_u32rtIndex] != val; myRTwriteMask[_u32rtIndex] = val; }

  // TODO: Make these protected/private again so myIsDirty and myCachedHash are always up to date
// protected:
  bool                         myAlphaToCoverageEnabled;
  bool                         myBlendStatePerRT;
  bool                         myAlphaSeparateBlend[Constants::kMaxNumRenderTargets];
  bool                         myBlendEnabled[Constants::kMaxNumRenderTargets];
  BlendInput                   mySrcBlend[Constants::kMaxNumRenderTargets];
  BlendInput                   myDestBlend[Constants::kMaxNumRenderTargets];
  BlendOp                      myBlendOp[Constants::kMaxNumRenderTargets];
  BlendInput                   mySrcBlendAlpha[Constants::kMaxNumRenderTargets];
  BlendInput                   myDestBlendAlpha[Constants::kMaxNumRenderTargets];
  BlendOp                      myBlendOpAlpha[Constants::kMaxNumRenderTargets];
  uint                       myRTwriteMask[Constants::kMaxNumRenderTargets];

  mutable uint64               myCachedHash;  // Needs to be modified from const GetHash()
  mutable bool                 myIsDirty;     // Needs to be modified from const GetHash()
//---------------------------------------------------------------------------//
};

} }  // end of namespace Fancy::Rendering
