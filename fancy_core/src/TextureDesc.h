#pragma once

#include "FancyCorePrerequisites.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  struct TextureDesc 
  {
    TextureDesc() : myIsExternalTexture(false), myInternalRefIndex(~0u) {}
    
    const bool operator==(const TextureDesc& anOther) const 
    {
      return  myIsExternalTexture == anOther.myIsExternalTexture
        &&    mySourcePath == anOther.mySourcePath
        &&    myInternalRefIndex == anOther.myInternalRefIndex;
    }

    uint64 GetHash() const
    {
      uint64 hash;

      MathUtil::hash_combine(hash, myIsExternalTexture);
      MathUtil::hash_combine(hash, MathUtil::hashFromString(mySourcePath));
      MathUtil::hash_combine(hash, myInternalRefIndex);

      return hash;
    }

    bool myIsExternalTexture;
    String mySourcePath; // The path to the original texture file (before caching - e.g. textures/barrel.jpg). Only valid for external textures
    uint32 myInternalRefIndex;  // The internal index of a texture used by the engine. Can be some semantics-enum defined by the renderingprocess (e.g. GBUFFER)
  };
//---------------------------------------------------------------------------//
} }  
