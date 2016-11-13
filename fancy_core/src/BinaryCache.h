#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"
#include "StringUtil.h"

namespace Fancy{ namespace Rendering{
struct TextureDesc;
} }

namespace Fancy { namespace Geometry {
struct MeshDesc;
class Mesh;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class BinaryCache
  {
    public:
  //---------------------------------------------------------------------------//
    static String getCacheFilePathAbs(const String& aPathInResources);
    static bool write(const SharedPtr<Rendering::Texture>& aTexture, const Rendering::TextureUploadData& someData);
    static bool write(Geometry::Mesh* aMesh, void** someVertexDatas, void** someIndexDatas);
    static bool read(SharedPtr<Rendering::Texture>* aTexture, uint64 aDescHash, uint32 aTimeStamp);
    static bool read(Geometry::Mesh** aMesh, uint64 aDescHash, uint32 aTimeStamp);
  //---------------------------------------------------------------------------//      
};
} }  // end of namespace Fancy::IO 

#endif  // INCLUDE_BINARYCACHE_H