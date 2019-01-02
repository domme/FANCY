#include "fancy_core_precompile.h"
#include "BlendState.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  BlendStateDesc BlendStateDesc::GetDefaultSolid()
  {
    return BlendStateDesc();
  }
//---------------------------------------------------------------------------//
  BlendStateDesc::BlendStateDesc() 
    : myAlphaToCoverageEnabled(false)
    , myBlendStatePerRT(false)
  {
    memset(myAlphaSeparateBlend, false, sizeof(myAlphaSeparateBlend));
    memset(myBlendEnabled, false, sizeof(myBlendEnabled));

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
    {
      mySrcBlend[i] = static_cast<uint>(BlendInput::ONE);
      myDestBlend[i] = static_cast<uint>(BlendInput::ONE);
      myBlendOp[i] = static_cast<uint>(BlendOp::ADD);
      mySrcBlendAlpha[i] = static_cast<uint>(BlendInput::ONE);
      myDestBlendAlpha[i] = static_cast<uint>(BlendInput::ONE);
      myBlendOpAlpha[i] = static_cast<uint>(BlendOp::ADD);
      myRTwriteMask[i] = UINT_MAX;
    }
  }
//---------------------------------------------------------------------------//
  bool BlendStateDesc::operator==(const BlendStateDesc& anOther) const
  {
    return memcmp(this, &anOther, sizeof(BlendStateDesc)) == 0;
  }
//---------------------------------------------------------------------------//
  uint64 BlendStateDesc::GetHash() const
  {
    uint64 hash = 0x0;
    MathUtil::hash_combine(hash, myAlphaToCoverageEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, myBlendStatePerRT ? 1u : 0u);

    for (uint i = 0; i < RenderConstants::kMaxNumRenderTargets; ++i)
    {
      MathUtil::hash_combine(hash, myAlphaSeparateBlend[i] ? 1u : 0u);
      MathUtil::hash_combine(hash, myBlendEnabled[i] ? 1u : 0u);
      MathUtil::hash_combine(hash, static_cast<uint>(mySrcBlend[i]));
      MathUtil::hash_combine(hash, static_cast<uint>(myDestBlend[i]));
      MathUtil::hash_combine(hash, static_cast<uint>(myBlendOp[i]));
      MathUtil::hash_combine(hash, static_cast<uint>(mySrcBlendAlpha[i]));
      MathUtil::hash_combine(hash, static_cast<uint>(myDestBlendAlpha[i]));
      MathUtil::hash_combine(hash, static_cast<uint>(myBlendOpAlpha[i]));
      MathUtil::hash_combine(hash, myRTwriteMask[i]);
    }

    return hash;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  BlendState::BlendState() 
    : myAlphaToCoverageEnabled(false)
    , myBlendStatePerRT(false)
  {
    memset(myAlphaSeparateBlend, false, sizeof(myAlphaSeparateBlend));
    memset(myBlendEnabled, false, sizeof(myBlendEnabled));

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
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

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
    {
      desc.myAlphaSeparateBlend[i] = myAlphaSeparateBlend[i];
      desc.myBlendEnabled[i] = myBlendEnabled[i];
      desc.mySrcBlend[i] = static_cast<uint>(mySrcBlend[i]);
      desc.myDestBlend[i] = static_cast<uint>(myDestBlend[i]);
      desc.myBlendOp[i] = static_cast<uint>(myBlendOp[i]);
      desc.mySrcBlendAlpha[i] = static_cast<uint>(mySrcBlendAlpha[i]);
      desc.myDestBlendAlpha[i] = static_cast<uint>(myDestBlendAlpha[i]);
      desc.myBlendOpAlpha[i] = static_cast<uint>(myBlendOpAlpha[i]);
      desc.myRTwriteMask[i] = static_cast<uint>(myRTwriteMask[i]);
    }
    
    return desc;
  }
//---------------------------------------------------------------------------//
  void BlendState::SetFromDescription(const BlendStateDesc& aDesc)
  {
    myAlphaToCoverageEnabled = aDesc.myAlphaToCoverageEnabled;
    myBlendStatePerRT = aDesc.myBlendStatePerRT;

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
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

  } 
//---------------------------------------------------------------------------//
	uint64 BlendState::GetHash() const
	{
    return GetDescription().GetHash();
	}
//---------------------------------------------------------------------------//
}