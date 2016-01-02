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
      depthStencilState.myStencilEnabled = false;
      depthStencilState.myDepthTestEnabled = true;
      depthStencilState.myDepthWriteEnabled = true;
      depthStencilState.myDepthCompFunc = Rendering::CompFunc::LESS;
      DepthStencilState::registerWithName(depthStencilState);
    }
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  DepthStencilState::DepthStencilState(const ObjectName& _name) :
    myName(_name),
    myDepthTestEnabled(true),
    myDepthWriteEnabled(true),
    myDepthCompFunc(CompFunc::LESS),
    myStencilEnabled(true),
    myTwoSidedStencil(false),
    myStencilRef(1),
    myStencilReadMask((Fancy::uint32)-1)
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
    return getHash() == clOther.getHash();
  }
//---------------------------------------------------------------------------//
  void DepthStencilState::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&myName, "myName");
  }
//---------------------------------------------------------------------------//
  uint DepthStencilState::getHash() const
  {
    uint hash = 0x0;
    MathUtil::hash_combine(hash, myDepthTestEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, myDepthWriteEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, (uint32) myDepthCompFunc);
    MathUtil::hash_combine(hash, myStencilEnabled ? 1u : 0u);
    MathUtil::hash_combine(hash, myTwoSidedStencil ? 1u : 0u);
    MathUtil::hash_combine(hash, myStencilRef);
    MathUtil::hash_combine(hash, myStencilReadMask);

    for (uint32 i = 0u; i < (uint32) FaceType::NUM; ++i)
    {
      MathUtil::hash_combine(hash, (uint32) myStencilCompFunc[i]);
      MathUtil::hash_combine(hash, myStencilWriteMask[i]);
      MathUtil::hash_combine(hash, (uint32) myStencilFailOp[i]);
      MathUtil::hash_combine(hash, (uint32) myStencilDepthFailOp[i]);
      MathUtil::hash_combine(hash, (uint32) myStencilPassOp[i]);
    }

    return hash;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering
