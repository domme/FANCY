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
class BlendState : public StaticManagedObject<BlendState> {

public:
  SERIALIZABLE(BlendState)

  static void init();

  explicit BlendState(const ObjectName& _name);
  ~BlendState() {}
  friend class PLATFORM_DEPENDENT_NAME(Renderer);
  bool operator==(const BlendState& clOther) const;
  
  const Fancy::ObjectName& getName() const { return myName; }
  static ObjectName getTypeName() { return _N(BlendState); }
  void serialize(IO::Serializer* aSerializer);
  
  uint getHash() const;

  bool getAlphaToCoverageEnabled() const { return myAlphaToCoverageEnabled; }
  void setAlphaToCoverageEnabled(bool val) { myAlphaToCoverageEnabled = val; }

  bool getBlendStatePerRT() const { return myBlendStatePerRT; }
  void setBlendStatePerRT(bool val) { myBlendStatePerRT = val;}
  
  bool getAlphaSeparateBlendEnabled(uint32 _u32rtIndex) const {return myAlphaSeparateBlend[_u32rtIndex]; }
  void setAlphaSeparateBlendEnabled(uint32 _u32rtIndex, bool val) {myAlphaSeparateBlend[_u32rtIndex] = val; }

  bool getBlendEnabled(uint32 _u32rtIndex) const {return myBlendEnabled[_u32rtIndex]; }
  void setBlendEnabled(uint32 _u32rtIndex, bool val) {myBlendEnabled[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlend(uint32 _u32rtIndex) const { return mySrcBlend[_u32rtIndex]; }
  void setSrcBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { mySrcBlend[_u32rtIndex] = val;}

  Fancy::Rendering::BlendInput getDestBlend(uint32 _u32rtIndex) const { return myDestBlend[_u32rtIndex]; }
  void setDestBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myDestBlend[_u32rtIndex] = val;}

  Fancy::Rendering::BlendOp getBlendOp(uint32 _u32rtIndex) const { return myBlendOp[_u32rtIndex]; }
  void setBlendOp(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { myBlendOp[_u32rtIndex] = val;}

  Fancy::Rendering::BlendInput getSrcBlendAlpha(uint32 _u32rtIndex) const { return mySrcBlendAlpha[_u32rtIndex]; }
  void setSrcBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { mySrcBlendAlpha[_u32rtIndex] = val;}

  Fancy::Rendering::BlendInput getDestBlendAlpha(uint32 _u32rtIndex) const { return myDestBlendAlpha[_u32rtIndex]; }
  void setDestBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { myDestBlendAlpha[_u32rtIndex] = val;}

  Fancy::Rendering::BlendOp getBlendOpAlpha(uint32 _u32rtIndex) const { return myBlendOpAlpha[_u32rtIndex]; }
  void setBlendOpAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { myBlendOpAlpha[_u32rtIndex] = val;}

  Fancy::uint32 getRTwriteMask(uint32 _u32rtIndex) const { return myRTwriteMask[_u32rtIndex]; }
  void setRTwriteMask(uint32 _u32rtIndex, Fancy::uint32 val) { myRTwriteMask[_u32rtIndex] = val;}

  ObjectName                   myName;
  bool                         myAlphaToCoverageEnabled;
  bool                         myBlendStatePerRT;
  bool                         myAlphaSeparateBlend[kMaxNumRenderTargets];
  bool                         myBlendEnabled[kMaxNumRenderTargets];
  BlendInput                   mySrcBlend[kMaxNumRenderTargets];
  BlendInput                   myDestBlend[kMaxNumRenderTargets];
  BlendOp                      myBlendOp[kMaxNumRenderTargets];
  BlendInput                   mySrcBlendAlpha[kMaxNumRenderTargets];
  BlendInput                   myDestBlendAlpha[kMaxNumRenderTargets];
  BlendOp                      myBlendOpAlpha[kMaxNumRenderTargets];
  uint32                       myRTwriteMask[kMaxNumRenderTargets];
//---------------------------------------------------------------------------//
};

} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_BLENDSTATE_H