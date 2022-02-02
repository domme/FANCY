#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/Ptr.h"

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
  class BinarySerializer;
//---------------------------------------------------------------------------//
  struct BinaryCache
  {
    static eastl::string GetCacheFilePathAbs(const char* aPathInResources);

    static void WriteScene(const char* aSourceFilePath, SceneData& aSceneData);
    static bool ReadScene(const char* aSourceFilePath, SceneData& aSceneData);

    static void WriteTextureData(const char* aSourceFilePath, TextureProperties& someTexProps, TextureData& aTextureData);
    static bool ReadTextureData(const char* aSourceFilePath, TextureProperties& someTexProps, TextureData& aTextureData);

  private:
    static bool HasValidDiskCache(const char* aPath);

    static bool SerializeScene(BinarySerializer& aSerializer, SceneData& aSceneData);
    static bool SerializeTextureData(BinarySerializer& aSerializer, TextureProperties& someTexProps, TextureData& aTextureData);
//---------------------------------------------------------------------------//      
  };
}