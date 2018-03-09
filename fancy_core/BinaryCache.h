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
    static bool WriteTexture(const Texture* aTexture, const TextureUploadData& someData);
    static bool WriteMesh(const Mesh* aMesh, const MeshDesc& aDesc, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas);

    static SharedPtr<Texture> ReadTexture(const TextureDesc& aDesc, uint64 aTimeStamp);
    static SharedPtr<Mesh> ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp);
  //---------------------------------------------------------------------------//      
};
}