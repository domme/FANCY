#include "BlendState.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  BlendStateDesc BlendStateDesc::GetDefaultSolid()
  {
    BlendStateDesc desc;

    desc.myAlphaToCoverageEnabled = false;
    desc.myBlendStatePerRT = false;
    memset(desc.myAlphaSeparateBlend, false, sizeof(desc.myAlphaSeparateBlend));
    memset(desc.myBlendEnabled, false, sizeof(desc.myBlendEnabled));

    for (uint32 i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
    {
      desc.mySrcBlend[i] = static_cast<uint32>(BlendInput::ONE);
      desc.myDestBlend[i] = static_cast<uint32>(BlendInput::ONE);
      desc.myBlendOp[i] = static_cast<uint32>(BlendOp::ADD);
      desc.mySrcBlendAlpha[i] = static_cast<uint32>(BlendInput::ONE);
      desc.myDestBlendAlpha[i] = static_cast<uint32>(BlendInput::ONE);
      desc.myBlendOpAlpha[i] = static_cast<uint32>(BlendOp::ADD);
      desc.myRTwriteMask[i] = UINT_MAX;
    }

    return desc;
  }
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
  uint64 BlendStateDesc::GetHash() const
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
  BlendState::BlendState() 
    : myAlphaToCoverageEnabled(false)
    , myBlendStatePerRT(false)
    , myCachedHash(0u)
    , myIsDirty(true)
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
    return GetHash() == clOther.GetHash();
  }
//---------------------------------------------------------------------------//
  bool BlendState::operator==(const BlendStateDesc& clOther) const
  {
    return GetHash() != clOther.GetHash();
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
  void BlendState::SetFromDescription(const BlendStateDesc& aDesc)
  {
    myAlphaToCoverageEnabled = aDesc.myAlphaToCoverageEnabled;
    myBlendStatePerRT = aDesc.myBlendStatePerRT;

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
    {
      myAlphaSeparateBlend[i] = aDesc.myAlphaSeparateBlend[i];
      myBlendEnabled[i] = aDesc.myBlendEnabled[i];
      mySrcBlend[i] = static_cast<BlendInput>(aDesc.mySrcBlend[i]);
      myDestBlend[i] = static_cast<BlendInput>(aDesc.myDestBlend[i]);
      myBlendOp[i] = static_cast<BlendOp>(aDesc.myBlendOp[i]);
      mySrcBlendAlpha[i] = static_cast<BlendInput>(aDesc.mySrcBlendAlpha[i]);
      myDestBlendAlpha[i] = static_cast<BlendInput>(aDesc.myDestBlendAlpha[i]);
      myBlendOpAlpha[i] = static_cast<BlendOp>(aDesc.myBlendOpAlpha[i]);
      myRTwriteMask[i] = aDesc.myRTwriteMask[i];
    }

    myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void BlendState::serialize(IO::Serializer* aSerializer)
  {
    // aSerializer->serialize(&myName, "myName");
  }
//---------------------------------------------------------------------------//
	uint64 BlendState::GetHash() const
	{
    if (!myIsDirty)
      return myCachedHash;

    myIsDirty = false;

    myCachedHash = GetDescription().GetHash();
    return myCachedHash;
	}
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering