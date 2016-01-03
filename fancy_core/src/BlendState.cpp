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
    myName(_name),
    myAlphaToCoverageEnabled(false),
    myBlendStatePerRT(false)
  {
    memset(myAlphaSeparateBlend, false, sizeof(myAlphaSeparateBlend));
    memset(myBlendEnabled, false, sizeof(myBlendEnabled));

    for (uint32 i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
    {
      mySrcBlend[i] = BlendInput::ONE;
      myDestBlend[i] = BlendInput::ONE;
      myBlendOp[i] = BlendOp::ADD;
      mySrcBlendAlpha[i] = BlendInput::ONE;
      myDestBlendAlpha[i] = BlendInput::ONE;
      myBlendOpAlpha[i] = BlendOp::ADD;
      myRTwriteMask[i] = UINT_MAX;
    }
  }
//---------------------------------------------------------------------------//
  bool BlendState::operator==( const BlendState& clOther ) const
  {
    return getHash() == clOther.getHash();
  }
//---------------------------------------------------------------------------//
  void BlendState::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myName, "myName");
  }
//---------------------------------------------------------------------------//
	uint BlendState::getHash() const
	{
		uint hash = 0x0;
		MathUtil::hash_combine(hash, myAlphaToCoverageEnabled ? 1u : 0u);
		MathUtil::hash_combine(hash, myBlendStatePerRT ? 1u : 0u);

		for (uint32 i = 0; i < Constants::kMaxNumRenderTargets; ++i)
		{
			MathUtil::hash_combine(hash, myAlphaSeparateBlend[i] ? 1u : 0u);
			MathUtil::hash_combine(hash, myBlendEnabled[i] ? 1u : 0u);
			MathUtil::hash_combine(hash, static_cast<uint32>(mySrcBlend[i]));
			MathUtil::hash_combine(hash, static_cast<uint32>(myDestBlend[i]));
			MathUtil::hash_combine(hash, static_cast<uint32>(myBlendOp[i]));
			MathUtil::hash_combine(hash, static_cast<uint32>(mySrcBlendAlpha[i]));
			MathUtil::hash_combine(hash, static_cast<uint32>(myDestBlendAlpha[i]));
			MathUtil::hash_combine(hash, static_cast<uint32>(myBlendOpAlpha[i]));
			MathUtil::hash_combine(hash, myRTwriteMask[i]);
		}

    return hash;
	}
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering