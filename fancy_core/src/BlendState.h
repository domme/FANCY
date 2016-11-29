#ifndef INCLUDE_BLENDSTATE_H
#define INCLUDE_BLENDSTATE_H

#include "ObjectName.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"
#include "Serializable.h"

namespace Fancy{namespace IO{
  class Serializer;
}}

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
    uint32 mySrcBlend[Constants::kMaxNumRenderTargets];
    uint32 myDestBlend[Constants::kMaxNumRenderTargets];
    uint32 myBlendOp[Constants::kMaxNumRenderTargets];
    uint32 mySrcBlendAlpha[Constants::kMaxNumRenderTargets];
    uint32 myDestBlendAlpha[Constants::kMaxNumRenderTargets];
    uint32 myBlendOpAlpha[Constants::kMaxNumRenderTargets];
    uint32 myRTwriteMask[Constants::kMaxNumRenderTargets];
  };
//---------------------------------------------------------------------------//
class BlendState : public StaticManagedObject<BlendState> {

public:
  SERIALIZABLE(BlendState)

  explicit BlendState();
  ~BlendState() {}
  friend class PLATFORM_DEPENDENT_NAME(RenderCore);
  bool operator==(const BlendState& clOther) const;
  bool operator==(const BlendStateDesc& clOther) const;

  BlendStateDesc GetDescription() const;
  void SetFromDescription(const BlendStateDesc& aDesc);

  uint64 GetHash() const;
  static ObjectName getTypeName() { return _N(BlendState); }
  void Serialize(IO::Serializer* aSerializer);

  bool getAlphaToCoverageEnabled() const { return myAlphaToCoverageEnabled; }
  void setAlphaToCoverageEnabled(bool val) { myIsDirty |= myAlphaToCoverageEnabled != val; myAlphaToCoverageEnabled = val; }

  bool getBlendStatePerRT() const { return myBlendStatePerRT; }
  void setBlendStatePerRT(bool val) { myIsDirty |= myBlendStatePerRT != val; myBlendStatePerRT = val; }
  
  bool getAlphaSeparateBlendEnabled(uint32 _u32rtIndex) const {return myAlphaSeparateBlend[_u32rtIndex]; }
  void setAlphaSeparateBlendEnabled(uint32 _u32rtIndex, bool val) { myIsDirty |= myAlphaSeparateBlend[_u32rtIndex] != val;  myAlphaSeparateBlend[_u32rtIndex] = val; }

  bool getBlendEnabled(uint32 _u32rtIndex) const {return myBlendEnabled[_u32rtIndex]; }
  void setBlendEnabled(uint32 _u32rtIndex, bool val) { myIsDirty |= myBlendEnabled[_u32rtIndex] != val;  myBlendEnabled[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlend(uint32 _u32rtIndex) const { return mySrcBlend[_u32rtIndex]; }
  void setSrcBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= mySrcBlend[_u32rtIndex] != val;  mySrcBlend[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getDestBlend(uint32 _u32rtIndex) const { return myDestBlend[_u32rtIndex]; }
  void setDestBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= myDestBlend[_u32rtIndex] != val;  myDestBlend[_u32rtIndex] = val; }

  Fancy::Rendering::BlendOp getBlendOp(uint32 _u32rtIndex) const { return myBlendOp[_u32rtIndex]; }
  void setBlendOp(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { myIsDirty |= myBlendOp[_u32rtIndex] != val;  myBlendOp[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlendAlpha(uint32 _u32rtIndex) const { return mySrcBlendAlpha[_u32rtIndex]; }
  void setSrcBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= mySrcBlendAlpha[_u32rtIndex] != val; mySrcBlendAlpha[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getDestBlendAlpha(uint32 _u32rtIndex) const { return myDestBlendAlpha[_u32rtIndex]; }
  void setDestBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myIsDirty |= myDestBlendAlpha[_u32rtIndex] != val;  myDestBlendAlpha[_u32rtIndex] = val; }

  Fancy::Rendering::BlendOp getBlendOpAlpha(uint32 _u32rtIndex) const { return myBlendOpAlpha[_u32rtIndex]; }
  void setBlendOpAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { myIsDirty |= myBlendOpAlpha[_u32rtIndex] != val; myBlendOpAlpha[_u32rtIndex] = val; }

  Fancy::uint32 getRTwriteMask(uint32 _u32rtIndex) const { return myRTwriteMask[_u32rtIndex]; }
  void setRTwriteMask(uint32 _u32rtIndex, Fancy::uint32 val) { myIsDirty |= myRTwriteMask[_u32rtIndex] != val; myRTwriteMask[_u32rtIndex] = val; }

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
  uint32                       myRTwriteMask[Constants::kMaxNumRenderTargets];

  mutable uint64               myCachedHash;  // Needs to be modified from const GetHash()
  mutable bool                 myIsDirty;     // Needs to be modified from const GetHash()
//---------------------------------------------------------------------------//
};

} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_BLENDSTATE_H