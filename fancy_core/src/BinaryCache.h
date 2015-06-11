#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"
#include "StringUtil.h"

namespace Fancy { namespace Geometry {
  class GeometryData;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class BinaryCache
  {
    public:
  //---------------------------------------------------------------------------//
    static String getCacheFilePathAbs(const ObjectName& aName);
    static bool write(Rendering::Texture* aTexture, void* someData, uint32 aDataSize);
    static bool write(Geometry::GeometryData* aGeometryData, void* someVertexData, 
      uint32 aVertexDataSize, void* someIndexData, uint32 anIndexDataSize);
    static bool load(Rendering::Texture** aTexture, const ObjectName& aName);
  //---------------------------------------------------------------------------//      
};
} }  // end of namespace Fancy::IO 

#endif  // INCLUDE_BINARYCACHE_H