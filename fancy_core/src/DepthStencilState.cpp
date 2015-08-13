#include "DepthStencilState.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  void DepthStencilState::init()
  {
    // Initialize common depthstencilstates
    // TODO: Do this from a file in the future?

    m_objectMap.clear();

    {
      DepthStencilState depthStencilState(_N(DepthStencilState_DefaultDepthState));
      depthStencilState.setStencilEnabled(false);
      depthStencilState.setDepthTestEnabled(true);
      depthStencilState.setDepthWriteEnabled(true);
      depthStencilState.setDepthCompFunc(Rendering::CompFunc::LESS);
      DepthStencilState::registerWithName(depthStencilState);
    }
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  DepthStencilState::DepthStencilState(const ObjectName& _name) :
    m_uHash(0u),
    m_Name(_name),
    m_bDepthTestEnabled(true),
    m_bDepthWriteEnabled(true),
    m_eDepthCompFunc(CompFunc::LESS),
    m_bStencilEnabled(true),
    m_bTwoSidedStencil(false),
    m_iStencilRef(1),
    m_uStencilReadMask((Fancy::uint32)-1)
  {
    m_eStencilCompFunc[(uint) FaceType::FRONT] = CompFunc::EQUAL;
    m_eStencilCompFunc[(uint) FaceType::BACK] = CompFunc::EQUAL;

    m_uStencilWriteMask[(uint) FaceType::FRONT] = (uint32)-1;
    m_uStencilWriteMask[(uint) FaceType::BACK] = (uint32)-1;

    m_eStencilFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    m_eStencilFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;

    m_eStencilDepthFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    m_eStencilDepthFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;

    m_eStencilPassOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    m_eStencilPassOp[(uint) FaceType::BACK]   = StencilOp::KEEP;
  }
  //---------------------------------------------------------------------------//
  bool DepthStencilState::operator==( const DepthStencilState& clOther ) const
  {
    return m_uHash == clOther.m_uHash;
  }
//---------------------------------------------------------------------------//
  void DepthStencilState::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
  }
//---------------------------------------------------------------------------//
  void DepthStencilState::updateHash()
  {
    uint hash = 0x0;
    MathUtil::hash_combine(hash, m_bDepthTestEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, m_bDepthWriteEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, (uint32) m_eDepthCompFunc);
    MathUtil::hash_combine(hash, m_bStencilEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, m_bTwoSidedStencil ? 1u : 0u);
    MathUtil::hash_combine(hash, m_iStencilRef);
    MathUtil::hash_combine(hash, m_uStencilReadMask);

    for (uint32 i = 0u; i < (uint32) FaceType::NUM; ++i)
    {
      MathUtil::hash_combine(hash, (uint32) m_eStencilCompFunc[i]);
      MathUtil::hash_combine(hash, m_uStencilWriteMask[i]);
      MathUtil::hash_combine(hash, (uint32) m_eStencilFailOp[i]);
      MathUtil::hash_combine(hash, (uint32) m_eStencilDepthFailOp[i]);
      MathUtil::hash_combine(hash, (uint32) m_eStencilPassOp[i]);
    }

    m_uHash = hash;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering
