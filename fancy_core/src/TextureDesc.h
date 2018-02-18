#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  struct TextureDesc
  {
    TextureDesc() : myIsExternalTexture(false), myInternalRefIndex(~0u) {}
    
    bool operator==(const TextureDesc& anOther) const 
    {
      return  myIsExternalTexture == anOther.myIsExternalTexture
        &&    mySourcePath == anOther.mySourcePath
        &&    myInternalRefIndex == anOther.myInternalRefIndex;
    }

    uint64 GetHash() const;
    bool IsEmpty() const { return mySourcePath.empty() && (myIsExternalTexture || myInternalRefIndex == ~0u); }

    bool myIsExternalTexture;
    String mySourcePath; // The path to the original texture file (before caching - e.g. textures/barrel.jpg). Only valid for external textures
    uint myInternalRefIndex;  // The internal index of a texture used by the engine. Can be some semantics-enum defined by the renderingprocess (e.g. GBUFFER)
  };
//---------------------------------------------------------------------------//
} }  
