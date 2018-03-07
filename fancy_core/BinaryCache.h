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
    static bool WriteMesh(const Mesh* aMesh, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas);

    static SharedPtr<Texture> ReadTexture(uint64 aDescHash, uint aTimeStamp);
    static SharedPtr<Mesh> ReadMesh(uint64 aDescHash, uint aTimeStamp);
  //---------------------------------------------------------------------------//      
};
}