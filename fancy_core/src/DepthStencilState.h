#ifndef INCLUDE_DEPTHSTENCILSTATE_H
#define INCLUDE_DEPTHSTENCILSTATE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Serializable.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class DepthStencilState : public StaticManagedObject<DepthStencilState>
  {
    friend class PLATFORM_DEPENDENT_NAME(Renderer);

  public:
    SERIALIZABLE(DepthStencilState)

    explicit DepthStencilState(const ObjectName& _name);
    ~DepthStencilState() {}
    static void init();
    bool operator==(const DepthStencilState& clOther) const;

    const ObjectName& getName() const {return m_Name;}
    static ObjectName getTypeName() { return _N(DepthStencilState); }
    void serialize(IO::Serializer* aSerializer);

    bool getDepthTestEnabled() const { return m_bDepthTestEnabled; }
    void setDepthTestEnabled(bool val) { m_bDepthTestEnabled = val; updateHash();}

    bool getDepthWriteEnabled() const { return m_bDepthWriteEnabled; }
    void setDepthWriteEnabled(bool val) { m_bDepthWriteEnabled = val; updateHash(); }

    Fancy::Rendering::CompFunc getDepthCompFunc() const { return m_eDepthCompFunc; }
    void setDepthCompFunc(Fancy::Rendering::CompFunc val) { m_eDepthCompFunc = val; updateHash(); }

    bool getStencilEnabled() const { return m_bStencilEnabled; }
    void setStencilEnabled(bool val) { m_bStencilEnabled = val; updateHash(); }

    bool getTwoSidedStencil() const { return m_bTwoSidedStencil; }
    void setTwoSidedStencil(bool val) { m_bTwoSidedStencil = val; updateHash(); }

    int getStencilRef() const { return m_iStencilRef; }
    void setStencilRef(int val) { m_iStencilRef = val; updateHash(); }

    Fancy::uint32 getStencilReadMask() const { return m_uStencilReadMask; }
    void setStencilReadMask(Fancy::uint32 val) { m_uStencilReadMask = val; updateHash(); }

    Fancy::Rendering::CompFunc getStencilCompFunc(uint32 _u32FaceType) const { return m_eStencilCompFunc[_u32FaceType]; }
    void setStencilCompFunc( uint32 _u32FaceType, Fancy::Rendering::CompFunc val) { m_eStencilCompFunc[_u32FaceType] = val; updateHash(); }

    Fancy::uint32 getStencilWriteMask(uint32 _u32FaceType) const { return m_uStencilWriteMask[_u32FaceType]; }
    void setStencilWriteMask(uint32 _u32FaceType, Fancy::uint32 val) { m_uStencilWriteMask[_u32FaceType] = val; updateHash(); }

    Fancy::Rendering::StencilOp getStencilFailOp(uint32 _u32FaceType) const { return m_eStencilFailOp[_u32FaceType]; }
    void setStencilFailOp(uint32 _u32FaceType, Fancy::Rendering::StencilOp val) { m_eStencilFailOp[_u32FaceType] = val; updateHash(); }

    Fancy::Rendering::StencilOp getStencilDepthFailOp(uint32 _u32FaceType) const { return m_eStencilDepthFailOp[_u32FaceType]; }
    void setStencilDepthFailOp(uint32 _u32FaceType, Fancy::Rendering::StencilOp val) { m_eStencilDepthFailOp[_u32FaceType] = val; updateHash(); }

    Fancy::Rendering::StencilOp getStencilPassOp(uint32 _u32FaceType) const { return m_eStencilPassOp[_u32FaceType]; }
    void setStencilPassOp(uint32 _u32FaceType, Fancy::Rendering::StencilOp val) { m_eStencilPassOp[_u32FaceType] = val; updateHash(); }

  private:
    void              updateHash();

    ObjectName        m_Name;
    uint32            m_uHash;
    bool              m_bDepthTestEnabled;
    bool              m_bDepthWriteEnabled;
    CompFunc          m_eDepthCompFunc;
    bool              m_bStencilEnabled;
    bool              m_bTwoSidedStencil;
    int               m_iStencilRef;
    uint32            m_uStencilReadMask;
    CompFunc          m_eStencilCompFunc[(uint32) FaceType::NUM];
    uint32            m_uStencilWriteMask[(uint32) FaceType::NUM];
    StencilOp         m_eStencilFailOp[(uint32) FaceType::NUM];
    StencilOp         m_eStencilDepthFailOp[(uint32) FaceType::NUM];
    StencilOp         m_eStencilPassOp[(uint32) FaceType::NUM];
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(DepthStencilState)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_DEPTHSTENCILSTATE_H