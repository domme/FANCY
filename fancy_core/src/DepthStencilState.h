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
    static DepthStencilStateDesc GetDefaultDepthNoStencil();

    DepthStencilStateDesc();
    bool operator==(const DepthStencilStateDesc& anOther) const;
    uint64 GetHash() const;

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

      DepthStencilState();
      ~DepthStencilState() {}
      bool operator==(const DepthStencilState& clOther) const;
      bool operator==(const DepthStencilStateDesc& aDesc) const;

      DepthStencilStateDesc GetDescription() const;
      void SetFromDescription(const DepthStencilStateDesc& aDesc);

      uint64 GetHash() const;
      static ObjectName getTypeName() { return _N(DepthStencilState); }
      void serialize(IO::Serializer* aSerializer);

      bool GetDepthTestEnabled() const { return myDepthTestEnabled; }
      void SetDepthTestEnabled(bool aDepthTestEnabled) { myIsDirty |= myDepthTestEnabled != aDepthTestEnabled; myDepthTestEnabled = aDepthTestEnabled; }
      bool GetDepthWriteEnabled() const { return myDepthWriteEnabled; }
      void SetDepthWriteEnabled(bool aDepthWriteEnabled) { myIsDirty |= myDepthWriteEnabled != aDepthWriteEnabled;  myDepthWriteEnabled = aDepthWriteEnabled; }
      CompFunc GetDepthCompFunc() const { return myDepthCompFunc; }
      void SetDepthCompFunc(CompFunc aDepthCompFunc) { myIsDirty |= myDepthCompFunc != aDepthCompFunc;  myDepthCompFunc = aDepthCompFunc; }
      bool GetStencilEnabled() const { return myStencilEnabled; }
      void SetStencilEnabled(bool aStencilEnabled) { myIsDirty |= myStencilEnabled != aStencilEnabled;  myStencilEnabled = aStencilEnabled; }
      bool GetTwoSidedStencil() const { return myTwoSidedStencil; }
      void SetTwoSidedStencil(bool aTwoSidedStencil) { myIsDirty |= myTwoSidedStencil != aTwoSidedStencil;  myTwoSidedStencil = aTwoSidedStencil; }
      int GetStencilRef() const { return myStencilRef; }
      void SetStencilRef(int aStencilRef) { myIsDirty |= myStencilRef != aStencilRef; myStencilRef = aStencilRef; }
      uint32 GetStencilReadMask() const { return myStencilReadMask; }
      void SetStencilReadMask(uint32 aStencilReadMask) { myIsDirty |= myStencilReadMask != aStencilReadMask;  myStencilReadMask = aStencilReadMask; }
      
      bool GetIsDirty() const { return myIsDirty; }
      void SetDirty() { myIsDirty = true; }

      

// TODO: Make these members private/protected to ensure validity of myIsDirty
  // protected:
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

      mutable bool myIsDirty;
      mutable uint64 myCachedHash;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(DepthStencilState)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_DEPTHSTENCILSTATE_H