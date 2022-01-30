#include "fancy_core_precompile.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "RenderCore.h"
#include "PathService.h"
#include "StringUtil.h"
#include "TextureProperties.h"
#include "GpuBuffer.h"
#include "Scene.h"
#include "BinarySerializer.h"

namespace Fancy {

#pragma region Serialize template specializations
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<VertexInputAttributeDesc>
  {
    void operator()(VertexInputAttributeDesc& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize((uint8*)&aVal, sizeof(aVal));
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<VertexBufferBindDesc>
  {
    void operator()(VertexBufferBindDesc& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize((uint8*)&aVal, sizeof(aVal));
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<VertexInputLayoutProperties> 
  {
    void operator()(VertexInputLayoutProperties& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myAttributes);
      aSerializer.Serialize(aVal.myBufferBindings);
    }
  };

//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<MeshDesc>
  {
    void operator()(MeshDesc& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myHash);
      aSerializer.Serialize(aVal.myName);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<MeshPartData>
  {
    void operator()(MeshPartData& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myVertexLayoutProperties);
      aSerializer.Serialize(aVal.myVertexData);
      aSerializer.Serialize(aVal.myIndexData);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<MeshData>
  {
    void operator()(MeshData& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myDesc);
      aSerializer.Serialize(aVal.myParts);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<MaterialDesc>
  {
    void operator()(MaterialDesc& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myParameters);
      aSerializer.Serialize(aVal.myTextures);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<SceneMeshInstance>
  {
    void operator()(SceneMeshInstance& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize((uint8*) &aVal, sizeof(aVal));
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<TextureProperties>
  {
    void operator()(TextureProperties& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myPath);
      aSerializer.Serialize(aVal.myDimension);
      aSerializer.Serialize(aVal.myWidth);
      aSerializer.Serialize(aVal.myHeight);
      aSerializer.Serialize(aVal.myDepthOrArraySize);
      aSerializer.Serialize(aVal.myFormat);
      aSerializer.Serialize(aVal.myAccessType);
      aSerializer.Serialize(aVal.myNumMipLevels);
      aSerializer.Serialize(aVal.bIsDepthStencil);
      aSerializer.Serialize(aVal.myIsShaderWritable);
      aSerializer.Serialize(aVal.myIsRenderTarget);
      aSerializer.Serialize(aVal.myPreferTypedFormat);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<TextureSubData>
  {
    void operator()(TextureSubData& aVal, BinarySerializer& aSerializer)
    {
      aSerializer.Serialize(aVal.myPixelSizeBytes);
      aSerializer.Serialize(aVal.myRowSizeBytes);
      aSerializer.Serialize(aVal.mySliceSizeBytes);
      aSerializer.Serialize(aVal.myTotalSizeBytes);
    }
  };
//---------------------------------------------------------------------------//
  template<>
  struct GetSerializeFunc<TextureData>
  {
    void operator()(TextureData& aVal, BinarySerializer& aSerializer)
    {
      if (aSerializer.IsWriting())  
      {
        // The data pointed to by the subdatas might actually point to different sources in memory - not necessarily to the data in TextureData::myData
        uint64 overallSizeBytes = 0ull;
        bool needsTempData = false;
        for (TextureSubData& subData : aVal.mySubDatas)
        {
          if (!needsTempData && 
            (overallSizeBytes > aVal.myData.size() || subData.myData != aVal.myData.data() + overallSizeBytes))
            needsTempData = true;

          overallSizeBytes += subData.myTotalSizeBytes;
        }

        if (needsTempData)
        {
          eastl::vector<uint8> rawTextureData(overallSizeBytes);
          uint8* dst = rawTextureData.data();
          for (TextureSubData& subData : aVal.mySubDatas)
          {
            memcpy(dst, subData.myData, subData.myTotalSizeBytes);
            dst += subData.myTotalSizeBytes;
          }

          aSerializer.Serialize(rawTextureData);
        }
        else
        {
          aSerializer.Serialize(aVal.myData);
        }
      }
      else
      {
        aSerializer.Serialize(aVal.myData);
      }
      
      aSerializer.Serialize(aVal.mySubDatas);

      if (aSerializer.IsReading())
      {
        uint8* dataPtr = aVal.myData.data();
        for (TextureSubData& subData : aVal.mySubDatas)
        {
          subData.myData = dataPtr;
          dataPtr += subData.myTotalSizeBytes;
        }
      }
    }
  };
//---------------------------------------------------------------------------//
#pragma endregion 

//---------------------------------------------------------------------------//
  const uint kCacheVersion = 5;
//---------------------------------------------------------------------------//
  eastl::string BinaryCache::GetCacheFilePathAbs(const char* aPathInResources)
  {
    return Path::GetUserDataPath() + "ResourceCache/" + aPathInResources + ".bin";
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteScene(const char* aSourceFilePath, SceneData& aSceneData)
  {
    const eastl::string cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), BinarySerializer::WRITE);
    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for write", cacheFilePath.c_str());
      return;
    }

    SerializeScene(serializer, aSceneData);
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadScene(const char* aSourceFilePath, SceneData& aSceneData)
  {
    if (!HasValidDiskCache(aSourceFilePath))
      return false;

    const eastl::string cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);

    BinarySerializer serializer(cacheFilePath.c_str(), BinarySerializer::READ);
    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for read", cacheFilePath.c_str());
      return false;
    }

    return SerializeScene(serializer, aSceneData);
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteTextureData(const char* aSourceFilePath, TextureProperties& someTexProps, TextureData& aTextureData)
  {
    const eastl::string cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), BinarySerializer::WRITE);
    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for write", cacheFilePath.c_str());
      return;
    }

    SerializeTextureData(serializer, someTexProps, aTextureData);
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadTextureData(const char* aSourceFilePath, TextureProperties& someTexProps, TextureData& aTextureData)
  {
    if (!HasValidDiskCache(aSourceFilePath))
      return false;

    const eastl::string cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);

    BinarySerializer serializer(cacheFilePath.c_str(), BinarySerializer::READ);
    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for read", cacheFilePath.c_str());
      return false;
    }

    return SerializeTextureData(serializer, someTexProps, aTextureData);
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::HasValidDiskCache(const char* aPath)
  {
    const eastl::string cacheFilePath = GetCacheFilePathAbs(aPath);

    eastl::string absSourcePath = Path::GetAbsolutePath(aPath);

    if (!Path::FileExists(absSourcePath.c_str()))
      return false;

    if (Path::GetFileWriteTime(cacheFilePath) < Path::GetFileWriteTime(absSourcePath))
      return false;

    return true;
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::SerializeScene(BinarySerializer& aSerializer, SceneData& aSceneData)
  {
    if (aSerializer.IsReading())
    {
      uint version = 0;
      aSerializer.Serialize(version);
      if (version != kCacheVersion)
        return false;
    }
    else
    {
      uint version = kCacheVersion;
      aSerializer.Serialize(version);
    }

    aSerializer.Serialize(aSceneData.myVertexInputLayoutProperties);
    aSerializer.Serialize(aSceneData.myMeshes);
    aSerializer.Serialize(aSceneData.myMaterials);
    aSerializer.Serialize(aSceneData.myInstances);

    return true;
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::SerializeTextureData(BinarySerializer& aSerializer, TextureProperties& someTexProps, TextureData& aTextureData)
  {
    if (aSerializer.IsReading())
    {
      uint version = 0;
      aSerializer.Serialize(version);
      if (version != kCacheVersion)
        return false;
    }
    else
    {
      uint version = kCacheVersion;
      aSerializer.Serialize(version);
    }

    aSerializer.Serialize(someTexProps);
    aSerializer.Serialize(aTextureData);

    return true;
  }
//---------------------------------------------------------------------------//
}

