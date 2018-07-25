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

    static String ToString(Ref aRef, uint anIndex = 0)
    {
      static const char* ourNameTable[] =
      {
#define TEXTURE_REF(name, ...) #name,
        #include "ResourceNameList.h"
#undef TEXTURE_REF
        "__Dummy__",  // Needed for intellisense to be happy
      };

      ASSERT(aRef < ARRAY_LENGTH(ourNameTable) - 1);  // -1 for dummy

      return StringFormat("%_%", ourNameTable[aRef], anIndex);
    }
  }
//---------------------------------------------------------------------------//
}
