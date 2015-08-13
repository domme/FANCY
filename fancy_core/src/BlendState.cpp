#include "BlendState.h"
#include "MathUtil.h"
#include "Serializer.h"

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

    for (uint32 i = 0u; i < kMaxNumRenderTargets; ++i)
    {
      m_eSrcBlend[i] = BlendInput::ONE;
      m_eDestBlend[i] = BlendInput::ONE;
      m_eBlendOp[i] = BlendOp::ADD;
      m_eSrcBlendAlpha[i] = BlendInput::ONE;
      m_eDestBlendAlpha[i] = BlendInput::ONE;
      m_eBlendOpAlpha[i] = BlendOp::ADD;
      m_uRTwriteMask[i] = UINT_MAX;
    }

    updateHash();
  }
//---------------------------------------------------------------------------//
  bool BlendState::operator==( const BlendState& clOther ) const
  {
    return m_uHash == clOther.m_uHash;
  }
//---------------------------------------------------------------------------//
  void BlendState::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_name, "m_name");
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
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eSrcBlend[i]));
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eDestBlend[i]));
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eBlendOp[i]));
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eSrcBlendAlpha[i]));
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eDestBlendAlpha[i]));
        MathUtil::hash_combine(hash, static_cast<uint32>(m_eBlendOpAlpha[i]));
        MathUtil::hash_combine(hash, m_uRTwriteMask[i]);
      }

      m_uHash = hash;
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering