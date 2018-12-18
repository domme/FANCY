#pragma once

#include "FancyCorePrerequisites.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct MeshDesc;
  class Mesh;
  struct MeshData;
//---------------------------------------------------------------------------//
  class BinaryCache
  {
    public:
  //---------------------------------------------------------------------------//
    static String getCacheFilePathAbs(const String& aPathInResources);
    static bool WriteTextureData(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas);
    static bool WriteMesh(const Mesh* aMesh, const MeshData* someMeshDatas, uint aNumMeshDatas);

    static SharedPtr<Texture> ReadTexture(const String& aPath, uint64 aTimeStamp);
    static SharedPtr<Mesh> ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp);
  //---------------------------------------------------------------------------//      
};
}