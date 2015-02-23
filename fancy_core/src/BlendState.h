#ifndef INCLUDE_BLENDSTATE_H
#define INCLUDE_BLENDSTATE_H

#include "ObjectName.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
class BlendState : public StaticManagedObject<BlendState> {

public:
  static void init();

 BlendState(const ObjectName& _name);
 ~BlendState() {}
 friend class PLATFORM_DEPENDENT_NAME(Renderer);
 bool operator==(const BlendState& clOther) const;
  
  Fancy::ObjectName getName() const { return m_name; }

  bool getAlphaToCoverageEnabled() const { return m_bAlphaToCoverageEnabled; }
  void setAlphaToCoverageEnabled(bool val) { m_bAlphaToCoverageEnabled = val; updateHash();}

  bool getBlendStatePerRT() const { return m_bBlendStatePerRT; }
  void setBlendStatePerRT(bool val) { m_bBlendStatePerRT = val; updateHash();}
  
  bool getAlphaSeparateBlendEnabled(uint32 _u32rtIndex) const {return m_bAlphaSeparateBlend[_u32rtIndex]; }
  void setAlphaSeparateBlendEnabled(uint32 _u32rtIndex, bool val) {m_bAlphaSeparateBlend[_u32rtIndex] = val; }

  bool getBlendEnabled(uint32 _u32rtIndex) const {return m_bBlendEnabled[_u32rtIndex]; }
  void setBlendEnabled(uint32 _u32rtIndex, bool val) {m_bBlendEnabled[_u32rtIndex] = val; }

  Fancy::Rendering::BlendInput getSrcBlend(uint32 _u32rtIndex) const { return m_eSrcBlend[_u32rtIndex]; }
  void setSrcBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { m_eSrcBlend[_u32rtIndex] = val; updateHash();}

  Fancy::Rendering::BlendInput getDestBlend(uint32 _u32rtIndex) const { return m_eDestBlend[_u32rtIndex]; }
  void setDestBlend(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { m_eDestBlend[_u32rtIndex] = val; updateHash();}

  Fancy::Rendering::BlendOp getBlendOp(uint32 _u32rtIndex) const { return m_eBlendOp[_u32rtIndex]; }
  void setBlendOp(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { m_eBlendOp[_u32rtIndex] = val; updateHash();}

  Fancy::Rendering::BlendInput getSrcBlendAlpha(uint32 _u32rtIndex) const { return m_eSrcBlendAlpha[_u32rtIndex]; }
  void setSrcBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { m_eSrcBlendAlpha[_u32rtIndex] = val; updateHash();}

  Fancy::Rendering::BlendInput getDestBlendAlpha(uint32 _u32rtIndex) const { return m_eDestBlendAlpha[_u32rtIndex]; }
  void setDestBlendAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendInput val) { m_eDestBlendAlpha[_u32rtIndex] = val; updateHash();}

  Fancy::Rendering::BlendOp getBlendOpAlpha(uint32 _u32rtIndex) const { return m_eBlendOpAlpha[_u32rtIndex]; }
  void setBlendOpAlpha(uint32 _u32rtIndex, Fancy::Rendering::BlendOp val) { m_eBlendOpAlpha[_u32rtIndex] = val; updateHash();}

  Fancy::uint32 getRTwriteMask(uint32 _u32rtIndex) const { return m_uRTwriteMask[_u32rtIndex]; }
  void setRTwriteMask(uint32 _u32rtIndex, Fancy::uint32 val) { m_uRTwriteMask[_u32rtIndex] = val; updateHash();}

private:
  void updateHash();
  
  uint                         m_uHash;
  ObjectName                   m_name;
  bool                         m_bAlphaToCoverageEnabled;
  bool                         m_bBlendStatePerRT;
  bool                         m_bAlphaSeparateBlend [kMaxNumRenderTargets];
  bool                         m_bBlendEnabled       [kMaxNumRenderTargets];
  BlendInput                   m_eSrcBlend           [kMaxNumRenderTargets];
  BlendInput                   m_eDestBlend          [kMaxNumRenderTargets];
  Rendering::BlendOp           m_eBlendOp            [kMaxNumRenderTargets];
  BlendInput                   m_eSrcBlendAlpha      [kMaxNumRenderTargets];
  BlendInput                   m_eDestBlendAlpha     [kMaxNumRenderTargets];
  Rendering::BlendOp           m_eBlendOpAlpha       [kMaxNumRenderTargets];
  uint32                       m_uRTwriteMask        [kMaxNumRenderTargets];
//---------------------------------------------------------------------------//
};

} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_BLENDSTATE_H