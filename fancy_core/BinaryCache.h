#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"

namespace Fancy {
  
  //---------------------------------------------------------------------------//
  struct MeshDesc;
  struct MeshData;
  struct MaterialDesc;
  struct TextureProperties;
  struct TextureSubData;
  struct TextureData;
  struct SceneData;
  struct VertexInputLayoutProperties;
//---------------------------------------------------------------------------//
  struct BinaryCache
  {
    struct BinarySerializer;

    static String GetCacheFilePathAbs(const char* aPathInResources);

    static void WriteScene(const char* aSourceFilePath, const SceneData& aSceneData);
    static bool ReadScene(const char* aSourceFilePath, SceneData& aSceneData);

    static void WriteTextureData(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas);
    static bool WriteMesh(const MeshData& aMeshData);

    static bool ReadTextureData(const String& aPath, uint64 aTimeStamp, TextureProperties& someTexPropsOut, TextureData& aTextureDataOut);
    static bool ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp, MeshData& aMeshDataOut);

  private:
    static void WriteTextureDataInternal(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas, BinarySerializer& aSerializer);

    static void WriteVertexInputLayout(const VertexInputLayoutProperties& aProps, BinarySerializer& aSerializer);
    static bool ReadVertexInputLayout(VertexInputLayoutProperties& aProps, BinarySerializer& aSerializer);

    static void WriteMeshDataInternal(const MeshData& aMeshData, BinarySerializer& aSerializer);
    static bool ReadMeshDataInternal(MeshData& aMeshData, BinarySerializer& aSerializer);

    static void WriteMaterialInternal(const MaterialDesc& aMaterialDesc, BinarySerializer& aSerializer);
    static bool ReadMaterialInternal(MaterialDesc& aMaterialDesc, BinarySerializer& aSerializer);
  //---------------------------------------------------------------------------//      
  };
}