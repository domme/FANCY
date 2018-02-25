#pragma once

#include "FancyCorePrerequisites.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct TextureDesc;
  struct MeshDesc;
  class Mesh;
//---------------------------------------------------------------------------//
  class BinaryCache
  {
    public:
  //---------------------------------------------------------------------------//
    static String getCacheFilePathAbs(const String& aPathInResources);
    static bool write(const SharedPtr<Texture>& aTexture, const TextureUploadData& someData);
    static bool write(const SharedPtr<Mesh>& aMesh, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas);
    static bool read(SharedPtr<Texture>* aTexture, uint64 aDescHash, uint aTimeStamp);
    static bool read(SharedPtr<Mesh>& aMesh, uint64 aDescHash, uint aTimeStamp);
  //---------------------------------------------------------------------------//      
};
}