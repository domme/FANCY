#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct MeshDesc;
  struct Mesh;
  struct MeshPartData;

  struct TextureProperties;
  struct TextureSubData;
  struct TextureData;
//---------------------------------------------------------------------------//
  namespace BinaryCache
  {
    String getCacheFilePathAbs(const String& aPathInResources);
    bool WriteTextureData(const TextureProperties& someTexProps , const TextureSubData* someSubDatas, uint aNumSubDatas);
    bool WriteMesh(const Mesh* aMesh, const MeshPartData* someMeshDatas, uint aNumMeshDatas);

    bool ReadTextureData(const String& aPath, uint64 aTimeStamp, TextureProperties& someTexPropsOut, TextureData& aTextureDataOut);
    SharedPtr<Mesh> ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp);
  //---------------------------------------------------------------------------//      
  };
}