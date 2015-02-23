#include "BlendState.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  void BlendState::init()
  {
    // Initialize common blendstates
    // TODO: Do this from a file in the future?

    m_objectMap.clear();
    
    {
      BlendState blendState(_N(BlendState_Solid));
      blendState.setBlendStatePerRT(false);
      blendState.setBlendEnabled(0u, false);
      blendState.setRTwriteMask(0u, UINT_MAX);
      BlendState::registerWithName(blendState);
    }

    // ... more to come
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  BlendState::BlendState(const ObjectName& _name) :
    m_uHash(0u),
    m_name(_name),
    m_bAlphaToCoverageEnabled(false),
    m_bBlendStatePerRT(false)
  {
    memset(m_bAlphaSeparateBlend, false, sizeof(m_bAlphaSeparateBlend));
    memset(m_bBlendEnabled, false, sizeof(m_bBlendEnabled));
    memset(m_eSrcBlend, (uint32) BlendInput::ONE, sizeof(m_eSrcBlend));
    memset(m_eDestBlend, (uint32) BlendInput::ONE, sizeof(m_eDestBlend));
    memset(m_eBlendOp, (uint32) BlendOp::ADD, sizeof(m_eBlendOp));
    memset(m_eSrcBlendAlpha, (uint32) BlendInput::ONE, sizeof(m_eSrcBlendAlpha));
    memset(m_eDestBlendAlpha, (uint32) BlendInput::ONE, sizeof(m_eDestBlendAlpha));
    memset(m_eBlendOpAlpha, (uint32) BlendOp::ADD, sizeof(m_eBlendOpAlpha));
    memset(m_uRTwriteMask, (uint32) -1, sizeof(m_uRTwriteMask));

    updateHash();
  }
//---------------------------------------------------------------------------//
  bool BlendState::operator==( const BlendState& clOther ) const
  {
    return m_uHash == clOther.m_uHash;
  }
//---------------------------------------------------------------------------//
  void BlendState::updateHash()
  {
      uint hash = 0x0;
      MathUtil::hash_combine(hash, m_bAlphaToCoverageEnabled ? 1u : 0u);
      MathUtil::hash_combine(hash, m_bBlendStatePerRT ? 1u : 0u);

      for (uint32 i = 0; i < kMaxNumRenderTargets; ++i)
      {
        MathUtil::hash_combine(hash, m_bAlphaSeparateBlend[i] ? 1u : 0u);
        MathUtil::hash_combine(hash, m_bBlendEnabled[i] ? 1u : 0u);
        MathUtil::hash_combine(hash, (uint32) m_eSrcBlend[i]);
        MathUtil::hash_combine(hash, (uint32) m_eDestBlend[i]);
        MathUtil::hash_combine(hash, (uint32) m_eBlendOp[i]);
        MathUtil::hash_combine(hash, (uint32) m_eSrcBlendAlpha[i]);
        MathUtil::hash_combine(hash, (uint32) m_eDestBlendAlpha[i]);
        MathUtil::hash_combine(hash, (uint32) m_eBlendOpAlpha[i]);
        MathUtil::hash_combine(hash, m_uRTwriteMask[i]);
      }
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering