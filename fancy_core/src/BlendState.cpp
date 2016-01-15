#include "BlendState.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  BlendStateDesc::BlendStateDesc() 
    : myAlphaToCoverageEnabled(false)
    , myBlendStatePerRT(false)
  {
    memset(myAlphaSeparateBlend, 0u, sizeof(myAlphaSeparateBlend));
  }

//---------------------------------------------------------------------------//
  bool BlendStateDesc::operator==(const BlendStateDesc& anOther) const
  {
    return memcmp(this, &anOther, sizeof(BlendStateDesc)) == 0;
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

  bool BlendState::operator==(const BlendStateDesc& clOther) const
  {
    const BlendStateDesc& desc = GetDescription();
    const BlendStateDesc& otherDesc = GetDescription();
    return MathUtil::hashFromGeneric(desc) == MathUtil::hashFromGeneric(otherDesc);
  }
  //---------------------------------------------------------------------------//
  BlendStateDesc BlendState::GetDescription() const
  {
    BlendStateDesc desc;

    desc.myAlphaToCoverageEnabled = myAlphaToCoverageEnabled;
    desc.myBlendStatePerRT = myBlendStatePerRT;

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
    {
      desc.myAlphaSeparateBlend[i] = myAlphaSeparateBlend[i];
      desc.myBlendEnabled[i] = myBlendEnabled[i];
      desc.mySrcBlend[i] = static_cast<uint32>(mySrcBlend[i]);
      desc.myDestBlend[i] = static_cast<uint32>(myDestBlend[i]);
      desc.myBlendOp[i] = static_cast<uint32>(myBlendOp[i]);
      desc.mySrcBlendAlpha[i] = static_cast<uint32>(mySrcBlendAlpha[i]);
      desc.myDestBlendAlpha[i] = static_cast<uint32>(myDestBlendAlpha[i]);
      desc.myBlendOpAlpha[i] = static_cast<uint32>(myBlendOpAlpha[i]);
      desc.myRTwriteMask[i] = static_cast<uint32>(myRTwriteMask[i]);
    }
    
    return desc;
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