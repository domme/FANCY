#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"
#include "FC_String.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct MeshDesc;
  class Mesh;
  struct MeshData;
  struct TextureProperties;
  struct TextureSubData;
  struct TextureData;
//---------------------------------------------------------------------------//
  namespace BinaryCache
  {
    String getCacheFilePathAbs(const String& aPathInResources);
    bool WriteTextureData(const TextureProperties& someTexProps , const TextureSubData* someSubDatas, uint aNumSubDatas);
    bool WriteMesh(const Mesh* aMesh, const MeshData* someMeshDatas, uint aNumMeshDatas);

    bool ReadTextureData(const String& aPath, uint64 aTimeStamp, TextureProperties& someTexPropsOut, TextureData& aTextureDataOut);
    SharedPtr<Mesh> ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp);
  //---------------------------------------------------------------------------//      
  };
}