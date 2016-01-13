#ifndef INCLUDE_DEPTHSTENCILSTATE_H
#define INCLUDE_DEPTHSTENCILSTATE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Serializable.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct DepthStencilStateDesc
  {
    bool myDepthTestEnabled;
    bool myDepthWriteEnabled;
    uint32 myDepthCompFunc;
    bool myStencilEnabled;
    bool myTwoSidedStencil;
    int myStencilRef;
    uint32 myStencilReadMask;
    uint32 myStencilCompFunc[(uint32)FaceType::NUM];
    uint32 myStencilWriteMask[(uint32)FaceType::NUM];
    uint32 myStencilFailOp[(uint32)FaceType::NUM];
    uint32 myStencilDepthFailOp[(uint32)FaceType::NUM];
    uint32 myStencilPassOp[(uint32)FaceType::NUM];
  };
//---------------------------------------------------------------------------//
  class DepthStencilState : public StaticManagedObject<DepthStencilState>
  {
    public:
      SERIALIZABLE(DepthStencilState)

      explicit DepthStencilState(const ObjectName& _name);
      ~DepthStencilState() {}
      bool operator==(const DepthStencilState& clOther) const;
      bool operator==(const DepthStencilStateDesc& aDesc) const;

      DepthStencilStateDesc GetDescription() const;

      const ObjectName& getName() const {return myName;}
      static ObjectName getTypeName() { return _N(DepthStencilState); }
      void serialize(IO::Serializer* aSerializer);

      uint getHash() const;

      ObjectName        myName;
      bool              myDepthTestEnabled;
      bool              myDepthWriteEnabled;
      CompFunc          myDepthCompFunc;
      bool              myStencilEnabled;
      bool              myTwoSidedStencil;
      int               myStencilRef;
      uint32            myStencilReadMask;
      CompFunc          myStencilCompFunc[(uint32)FaceType::NUM];
      uint32            myStencilWriteMask[(uint32)FaceType::NUM];
      StencilOp         myStencilFailOp[(uint32)FaceType::NUM];
      StencilOp         myStencilDepthFailOp[(uint32)FaceType::NUM];
      StencilOp         myStencilPassOp[(uint32)FaceType::NUM];
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(DepthStencilState)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_DEPTHSTENCILSTATE_H