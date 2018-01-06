#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "StaticManagedObject.h"
#include "Serializable.h"
#include "DescriptionBase.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct DepthStencilStateDesc : public DescriptionBase
  {
    static DepthStencilStateDesc GetDefaultDepthNoStencil();

    DepthStencilStateDesc();
    ~DepthStencilStateDesc() override {}

    bool IsEmpty() const override { return false; }

    ObjectName GetTypeName() const override { return _N(DepthStencilState); }
    void Serialize(IO::Serializer* aSerializer) override;
    
    bool operator==(const DepthStencilStateDesc& anOther) const;
    uint64 GetHash() const override;

    bool myDepthTestEnabled;
    bool myDepthWriteEnabled;
    uint myDepthCompFunc;
    bool myStencilEnabled;
    bool myTwoSidedStencil;
    int myStencilRef;
    uint myStencilReadMask;
    uint myStencilCompFunc[(uint)FaceType::NUM];
    uint myStencilWriteMask[(uint)FaceType::NUM];
    uint myStencilFailOp[(uint)FaceType::NUM];
    uint myStencilDepthFailOp[(uint)FaceType::NUM];
    uint myStencilPassOp[(uint)FaceType::NUM];
  };
//---------------------------------------------------------------------------//
  class DepthStencilState
  {
  public:
      SERIALIZABLE_RESOURCE(DepthStencilState)

      DepthStencilState();
      ~DepthStencilState() {}
      bool operator==(const DepthStencilState& clOther) const;
      bool operator==(const DepthStencilStateDesc& aDesc) const;

      DepthStencilStateDesc GetDescription() const;
      void SetFromDescription(const DepthStencilStateDesc& aDesc);

      uint64 GetHash() const;

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
      uint GetStencilReadMask() const { return myStencilReadMask; }
      void SetStencilReadMask(uint aStencilReadMask) { myIsDirty |= myStencilReadMask != aStencilReadMask;  myStencilReadMask = aStencilReadMask; }
      
      bool GetIsDirty() const { return myIsDirty; }
      void SetDirty() const { myIsDirty = true; }

// TODO: Make these members private/protected to ensure validity of myIsDirty
  // protected:
      bool              myDepthTestEnabled;
      bool              myDepthWriteEnabled;
      CompFunc          myDepthCompFunc;
      bool              myStencilEnabled;
      bool              myTwoSidedStencil;
      int               myStencilRef;
      uint            myStencilReadMask;
      CompFunc          myStencilCompFunc[(uint)FaceType::NUM];
      uint            myStencilWriteMask[(uint)FaceType::NUM];
      StencilOp         myStencilFailOp[(uint)FaceType::NUM];
      StencilOp         myStencilDepthFailOp[(uint)FaceType::NUM];
      StencilOp         myStencilPassOp[(uint)FaceType::NUM];

      mutable bool myIsDirty;
      mutable uint64 myCachedHash;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(DepthStencilState)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering