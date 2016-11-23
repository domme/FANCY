#pragma once

#include "FancyCorePrerequisites.h"
#include "ResourceDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  struct TextureDesc : public ResourceDesc
  {
    TextureDesc() : myIsExternalTexture(false), myInternalRefIndex(~0u) {}
    ~TextureDesc() override {}
    
    bool operator==(const TextureDesc& anOther) const 
    {
      return  myIsExternalTexture == anOther.myIsExternalTexture
        &&    mySourcePath == anOther.mySourcePath
        &&    myInternalRefIndex == anOther.myInternalRefIndex;
    }

    ObjectName GetTypeName() const override { return _N(Texture); }
    uint64 GetHash() const override;
    void Serialize(IO::Serializer* aSerializer) override;


    bool myIsExternalTexture;
    String mySourcePath; // The path to the original texture file (before caching - e.g. textures/barrel.jpg). Only valid for external textures
    uint32 myInternalRefIndex;  // The internal index of a texture used by the engine. Can be some semantics-enum defined by the renderingprocess (e.g. GBUFFER)
  };
//---------------------------------------------------------------------------//
} }  
