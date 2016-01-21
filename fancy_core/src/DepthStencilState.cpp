#include "DepthStencilState.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  DepthStencilStateDesc DepthStencilStateDesc::GetDefaultDepthNoStencil()
  {
    DepthStencilStateDesc desc;
    desc.myDepthTestEnabled = true;
    desc.myDepthWriteEnabled = true;
    desc.myDepthCompFunc = static_cast<uint32>(CompFunc::LESS);
    desc.myStencilEnabled = false;
    desc.myTwoSidedStencil = false;
    desc.myStencilRef = 0;
    desc.myStencilReadMask = 0u;
    memset(desc.myStencilCompFunc, 0u, sizeof(desc.myStencilCompFunc));
    memset(desc.myStencilWriteMask, 0u, sizeof(desc.myStencilWriteMask));
    memset(desc.myStencilFailOp, 0u, sizeof(desc.myStencilFailOp));
    memset(desc.myStencilDepthFailOp, 0u, sizeof(desc.myStencilDepthFailOp));
    memset(desc.myStencilPassOp, 0u, sizeof(desc.myStencilPassOp));
    return desc;
  }
//---------------------------------------------------------------------------//
  DepthStencilStateDesc::DepthStencilStateDesc() 
    : myDepthTestEnabled(false)
    , myDepthWriteEnabled(false)
    , myDepthCompFunc(0u)
    , myStencilEnabled(false)
    , myTwoSidedStencil(false)
    , myStencilRef(0)
    , myStencilReadMask(0u)
  {
    memset(myStencilCompFunc, 0u, sizeof(myStencilCompFunc));
    memset(myStencilWriteMask, 0u, sizeof(myStencilWriteMask));
    memset(myStencilFailOp, 0u, sizeof(myStencilFailOp));
    memset(myStencilDepthFailOp, 0u, sizeof(myStencilDepthFailOp));
    memset(myStencilPassOp, 0u, sizeof(myStencilPassOp));
  }
//---------------------------------------------------------------------------//
  bool DepthStencilStateDesc::operator==(const DepthStencilStateDesc& anOther) const
  {
    return memcmp(this, &anOther, sizeof(DepthStencilState)) == 0;
  }
//---------------------------------------------------------------------------//
  uint64 DepthStencilStateDesc::GetHash() const
  {
    uint hash = 0x0;
    MathUtil::hash_combine(hash, myDepthTestEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, myDepthWriteEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, (uint32)myDepthCompFunc);
    MathUtil::hash_combine(hash, myStencilEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, myTwoSidedStencil ? 1u : 0u);
    MathUtil::hash_combine(hash, myStencilRef);
    MathUtil::hash_combine(hash, myStencilReadMask);

    for (uint32 i = 0u; i < (uint32)FaceType::NUM; ++i)
    {
      MathUtil::hash_combine(hash, (uint32)myStencilCompFunc[i]);
      MathUtil::hash_combine(hash, myStencilWriteMask[i]);
      MathUtil::hash_combine(hash, (uint32)myStencilFailOp[i]);
      MathUtil::hash_combine(hash, (uint32)myStencilDepthFailOp[i]);
      MathUtil::hash_combine(hash, (uint32)myStencilPassOp[i]);
    }

    return hash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  DepthStencilState::DepthStencilState(const ObjectName& _name) 
    : myName(_name)
    , myDepthTestEnabled(true)
    , myDepthWriteEnabled(true)
    , myDepthCompFunc(CompFunc::LESS)
    , myStencilEnabled(true)
    , myTwoSidedStencil(false)
    , myStencilRef(1)
    , myStencilReadMask((Fancy::uint32)-1)
    , myCachedHash(0u)
    , myIsDirty(true)
  {
    myStencilCompFunc[(uint) FaceType::FRONT] = CompFunc::EQUAL;
    myStencilCompFunc[(uint) FaceType::BACK] = CompFunc::EQUAL;
    myStencilWriteMask[(uint) FaceType::FRONT] = (uint32)-1;
    myStencilWriteMask[(uint) FaceType::BACK] = (uint32)-1;
    myStencilFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    myStencilFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;
    myStencilDepthFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    myStencilDepthFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;
    myStencilPassOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
    myStencilPassOp[(uint) FaceType::BACK]   = StencilOp::KEEP;
  }
  //---------------------------------------------------------------------------//
  bool DepthStencilState::operator==( const DepthStencilState& clOther ) const
  {
    return GetHash() == clOther.GetHash();
  }
//---------------------------------------------------------------------------//
  bool DepthStencilState::operator==(const DepthStencilStateDesc& aDesc) const 
  {
    const DepthStencilStateDesc& desc = GetDescription();
    return desc == aDesc;
  }
//---------------------------------------------------------------------------//
  DepthStencilStateDesc DepthStencilState::GetDescription() const
  {
    DepthStencilStateDesc desc;
    desc.myDepthTestEnabled = myDepthTestEnabled;
    desc.myDepthWriteEnabled = myDepthWriteEnabled;
    desc.myDepthCompFunc = static_cast<uint32>(myDepthCompFunc);
    desc.myStencilEnabled = myStencilEnabled;
    desc.myTwoSidedStencil = myTwoSidedStencil;
    desc.myStencilRef = myStencilRef;
    desc.myStencilReadMask = myStencilReadMask;

    for (uint32 i = 0u; i < static_cast<uint32>(FaceType::NUM); ++i)
    {
      desc.myStencilCompFunc[i] = static_cast<uint32>(myStencilCompFunc[i]);
      desc.myStencilWriteMask[i] = myStencilWriteMask[i];
      desc.myStencilFailOp[i] = static_cast<uint32>(myStencilFailOp[i]);
      desc.myStencilDepthFailOp[i] = static_cast<uint32>(myStencilDepthFailOp[i]);
      desc.myStencilPassOp[i] = static_cast<uint32>(myStencilPassOp[i]);
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void DepthStencilState::SetFromDescription(const DepthStencilStateDesc& aDesc)
  {
    myDepthTestEnabled = aDesc.myDepthTestEnabled;
    myDepthWriteEnabled = aDesc.myDepthWriteEnabled;
    myDepthCompFunc = static_cast<CompFunc>(aDesc.myDepthCompFunc);
    myStencilEnabled = aDesc.myStencilEnabled;
    myTwoSidedStencil = aDesc.myTwoSidedStencil;
    myStencilRef = aDesc.myStencilRef;
    myStencilReadMask = aDesc.myStencilReadMask;

    for (uint32 i = 0u; i < static_cast<uint32>(FaceType::NUM); ++i)
    {
      myStencilCompFunc[i] = static_cast<CompFunc>(aDesc.myStencilCompFunc[i]);
      myStencilWriteMask[i] = aDesc.myStencilWriteMask[i];
      myStencilFailOp[i] = static_cast<StencilOp>(aDesc.myStencilFailOp[i]);
      myStencilDepthFailOp[i] = static_cast<StencilOp>(aDesc.myStencilDepthFailOp[i]);
      myStencilPassOp[i] = static_cast<StencilOp>(aDesc.myStencilPassOp[i]);
    }

    myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void DepthStencilState::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myName, "myName");
  }
//---------------------------------------------------------------------------//
  uint DepthStencilState::GetHash() const
  {
    if (!myIsDirty)
      return myCachedHash;

    myIsDirty = false;
    myCachedHash = GetDescription().GetHash();

    return myCachedHash;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering
