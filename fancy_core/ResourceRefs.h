#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace TextureRef
  {
    enum Ref
    {
#define TEXTURE_REF(name, id, array) name = id, LAST_##name = id + (array ? array-1 : 0),
      #include "ResourceNameList.h"
#undef TEXTURE_REF
      NUM_LOCAL_TEXTURES = LAST_LOCAL_TEXTURE + 1 - LOCAL_TEXTURE,
      NUM_GLOBAL_TEXTURES = 13 - LAST_LOCAL_TEXTURE,
    };

    static eastl::string ToString(Ref aRef, uint anIndex = 0)
    {
#define TEXTURE_REF(name, ...) case name: return (const char*) StaticString<64>("%s_%d", #name, anIndex);
      switch(aRef)
      {
        #include "ResourceNameList.h"
        default: ASSERT(false); return "";
      };
#undef TEXTURE_REF
    }
  }
//---------------------------------------------------------------------------//
}
