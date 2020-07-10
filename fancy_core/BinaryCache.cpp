#include "fancy_core_precompile.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "RenderCore.h"
#include "PathService.h"
#include "StringUtil.h"
#include "DynamicArray.h"
#include "TextureProperties.h"
#include "GpuBuffer.h"
#include "Scene.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  const uint kCacheVersion = 4;
//---------------------------------------------------------------------------//
  enum SERIALIZE_MODE
  {
    READ, WRITE
  };

  struct BinaryCache::BinarySerializer
  {
    BinarySerializer(const char* aPath, SERIALIZE_MODE aMode)
      : myMode(aMode)
      , myStream(std::fstream(aPath, std::ios::binary | (aMode == WRITE ? std::ios::out : std::ios::in))) 
    { }

    bool IsGood() const { return myStream.good(); }

    void Write(const uint8* someData, uint64 aDataSize);
    void Read(uint8* someData, uint64 aDataSize);
    
    template<class T> void Write(const T& aVal);
    template<class T> void Read(T& aVal);

    SERIALIZE_MODE myMode;
    std::fstream myStream;
  };
//---------------------------------------------------------------------------//
  void BinaryCache::BinarySerializer::Write(const uint8* someData, uint64 aDataSize)
  {
    ASSERT(myMode == WRITE);
    myStream.write((const char*) someData, aDataSize);
  }
//---------------------------------------------------------------------------//
  void BinaryCache::BinarySerializer::Read(uint8* someData, uint64 aDataSize)
  {
    ASSERT(myMode == READ);
    myStream.read((char*)someData, aDataSize);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinaryCache::BinarySerializer::Write(const T& aVal)
  {
    ASSERT(myMode == WRITE);
    Write((const uint8*)(&aVal), sizeof(T));
  }
//---------------------------------------------------------------------------//
  template<>
  void BinaryCache::BinarySerializer::Write<String>(const String& aVal)
  {
    ASSERT(myMode == WRITE);
    const char* name_cstr = aVal.c_str();
    const uint64 name_size = static_cast<uint>(aVal.size()) + 1u; // size + '/0'
    Write(name_size);
    Write((const uint8*)name_cstr, name_size);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinaryCache::BinarySerializer::Read(T& aVal)
  {
    ASSERT(myMode == READ);
    myStream.read((char*) &aVal, sizeof(T));
  }
//---------------------------------------------------------------------------//
  template<>
  void BinaryCache::BinarySerializer::Read<String>(String& aVal)
  {
    ASSERT(myMode == READ);
    uint64 name_size;
    Read(name_size);
    
    const uint kExpectedMaxLength = 64u;
    char buf[kExpectedMaxLength];
    char* name_cstr = buf;

    if (name_size > kExpectedMaxLength)
      name_cstr = new char[name_size];

    myStream.read(name_cstr, name_size);
    aVal = name_cstr;

    if (name_size > kExpectedMaxLength)
      delete[] name_cstr;
  }
//---------------------------------------------------------------------------//
  String BinaryCache::GetCacheFilePathAbs(const char* aPathInResources)
  {
    return Path::GetUserDataPath() + "ResourceCache/" + aPathInResources + ".bin";
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteScene(const char* aSourceFilePath, const SceneData& aSceneData)
  {
    const String cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for write", cacheFilePath.c_str());
      return;
    }
  
    serializer.Write(kCacheVersion);
    WriteVertexInputLayout(aSceneData.myVertexInputLayoutProperties, serializer);

    serializer.Write((uint)aSceneData.myMeshes.size());
    for (const MeshData& meshData : aSceneData.myMeshes)
      WriteMeshDataInternal(meshData, serializer);

    serializer.Write((uint)aSceneData.myMaterials.size());
    for (const MaterialDesc& materialDesc : aSceneData.myMaterials)
      WriteMaterialInternal(materialDesc, serializer);

    serializer.Write((uint)aSceneData.myInstances.size());
    for (const SceneMeshInstance& meshInstance : aSceneData.myInstances)
      serializer.Write(meshInstance);
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadScene(const char* aSourceFilePath, SceneData& aSceneData)
  {
    const String cacheFilePath = GetCacheFilePathAbs(aSourceFilePath);

    bool foundSourceFile = false;
    String absSourcePath = Path::GetAbsoluteResourcePath(aSourceFilePath, &foundSourceFile);

    if (!foundSourceFile)
      return false;

    if (Path::GetFileWriteTime(cacheFilePath) < Path::GetFileWriteTime(absSourcePath))
      return false;

    BinarySerializer serializer(cacheFilePath.c_str(), READ);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open scene cache file path %s for read", cacheFilePath.c_str());
      return false;
    }

    uint cacheVersion;
    serializer.Read(cacheVersion);

    if (cacheVersion != kCacheVersion)
      return false;

    bool success = true;

    success &= ReadVertexInputLayout(aSceneData.myVertexInputLayoutProperties, serializer);

    uint numMeshes = 0;
    serializer.Read(numMeshes);
    aSceneData.myMeshes.resize(numMeshes);
    for (MeshData& meshData : aSceneData.myMeshes)
      success &= ReadMeshDataInternal(meshData, serializer);

    uint numMaterials = 0;
    serializer.Read(numMaterials);
    aSceneData.myMaterials.resize(numMaterials);
    for (MaterialDesc& materialDesc : aSceneData.myMaterials)
      success &= ReadMaterialInternal(materialDesc, serializer);

    uint numInstances = 0;
    serializer.Read(numInstances);
    aSceneData.myInstances.resize(numInstances);
    for (SceneMeshInstance& meshInstance : aSceneData.myInstances)
      serializer.Read(meshInstance);

    return success;
  }
//---------------------------------------------------------------------------//  
  void BinaryCache::WriteTextureData(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas)
  {
    const String cacheFilePath = GetCacheFilePathAbs(someTexProps.path.c_str());
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open texture cache file path %s for write", cacheFilePath.c_str());
      return;
    }

    WriteTextureDataInternal(someTexProps, someSubDatas, aNumSubDatas, serializer);
  }
//---------------------------------------------------------------------------//  
  void BinaryCache::WriteTextureDataInternal(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas, BinarySerializer& aSerializer)
  {
    aSerializer.Write(kCacheVersion);
    aSerializer.Write(someTexProps.path);

    aSerializer.Write((uint)someTexProps.myDimension);
    aSerializer.Write(someTexProps.myWidth);
    aSerializer.Write(someTexProps.myHeight);
    aSerializer.Write(someTexProps.myDepthOrArraySize);
    aSerializer.Write((uint)someTexProps.myAccessType);
    aSerializer.Write((uint)someTexProps.myFormat);
    aSerializer.Write(someTexProps.myNumMipLevels);

    aSerializer.Write(aNumSubDatas);

    uint64 totalPixelSize = 0;
    for (uint i = 0u; i < aNumSubDatas; ++i)
      totalPixelSize += someSubDatas[i].myTotalSizeBytes;

    aSerializer.Write(totalPixelSize);

    for (uint i = 0u; i < aNumSubDatas; ++i)
    {
      const TextureSubData& someData = someSubDatas[i];
      aSerializer.Write(someData.myPixelSizeBytes);
      aSerializer.Write(someData.myRowSizeBytes);
      aSerializer.Write(someData.mySliceSizeBytes);
      aSerializer.Write(someData.myTotalSizeBytes);
      aSerializer.Write(reinterpret_cast<const uint8*>(someData.myData), someData.myTotalSizeBytes);
    }
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteVertexInputLayout(const VertexInputLayoutProperties& aProps, BinarySerializer& aSerializer)
  {
    const StaticArray<VertexInputAttributeDesc, 16>& vertexAttributes = aProps.myAttributes;
    const uint numVertexElements = vertexAttributes.Size();
    aSerializer.Write(numVertexElements);

    for (uint iVertexElem = 0u; iVertexElem < vertexAttributes.Size(); ++iVertexElem)
    {
      const VertexInputAttributeDesc& vertexElement = vertexAttributes[iVertexElem];
      aSerializer.Write((uint)vertexElement.myFormat);
      aSerializer.Write(vertexElement.myBufferIndex);
      aSerializer.Write((uint)vertexElement.mySemantic);
      aSerializer.Write(vertexElement.mySemanticIndex);
    }

    const StaticArray<VertexBufferBindDesc, 16>& vertexBuffers = aProps.myBufferBindings;
    aSerializer.Write(vertexBuffers.Size());

    for (uint iBuffer = 0u; iBuffer < vertexBuffers.Size(); ++iBuffer)
    {
      const VertexBufferBindDesc& buffer = vertexBuffers[iBuffer];
      aSerializer.Write((uint)buffer.myInputRate);
      aSerializer.Write(buffer.myStride);
    }
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadVertexInputLayout(VertexInputLayoutProperties& aProps, BinarySerializer& aSerializer)
  {
    uint numVertexElements;
    aSerializer.Read(numVertexElements);

    for (uint iVertexElem = 0u; iVertexElem < numVertexElements; ++iVertexElem)
    {
      VertexInputAttributeDesc& vertexElement = aProps.myAttributes.Add();
      uint format;
      aSerializer.Read(format);
      vertexElement.myFormat = (DataFormat)format;
      
      aSerializer.Read(vertexElement.myBufferIndex);

      uint semantic;
      aSerializer.Read(semantic);
      vertexElement.mySemantic = (VertexAttributeSemantic)semantic;

      aSerializer.Read(vertexElement.mySemanticIndex);
    }

    uint numVertexBuffers;
    aSerializer.Read(numVertexBuffers);

    for (uint iBuffer = 0u; iBuffer < numVertexBuffers; ++iBuffer)
    {
      VertexBufferBindDesc& buffer = aProps.myBufferBindings.Add();
      uint inputRate;
      aSerializer.Read(inputRate);

      buffer.myInputRate = (VertexInputRate)inputRate;
      aSerializer.Read(buffer.myStride);
    }

    return aSerializer.IsGood();
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteMeshDataInternal(const MeshData& aMeshData, BinarySerializer& aSerializer)
  {
    aSerializer.Write(kCacheVersion);
    aSerializer.Write(aMeshData.myDesc.myHash);
    aSerializer.Write(aMeshData.myDesc.myName);
    aSerializer.Write((uint)aMeshData.myParts.size());

    for (uint i = 0u; i < (uint)aMeshData.myParts.size(); ++i)
    {
      const MeshPartData& meshPartData = aMeshData.myParts[i];

      const VertexInputLayoutProperties& vertexLayout = meshPartData.myVertexLayoutProperties;
      WriteVertexInputLayout(vertexLayout, aSerializer);

      // Vertex data
      {
        uint64 bufferSize = DYN_ARRAY_BYTESIZE(aMeshData.myParts[i].myVertexData);
        aSerializer.Write(bufferSize);
        aSerializer.Write(reinterpret_cast<const uint8*>(aMeshData.myParts[i].myVertexData.data()), bufferSize);
      }

      // Index data
      {
        uint64 bufferSize = DYN_ARRAY_BYTESIZE(aMeshData.myParts[i].myIndexData);
        aSerializer.Write(bufferSize);
        aSerializer.Write(reinterpret_cast<const uint8*>(aMeshData.myParts[i].myIndexData.data()), bufferSize);
      }
    }
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadMeshDataInternal(MeshData& aMeshData, BinarySerializer& aSerializer)
  {
    uint cacheVersion;
    aSerializer.Read(cacheVersion);
    if (cacheVersion != kCacheVersion)
      return false;

    aSerializer.Read(aMeshData.myDesc.myHash);
    aSerializer.Read(aMeshData.myDesc.myName);

    uint numMeshParts;
    aSerializer.Read(numMeshParts);

    aMeshData.myParts.resize(numMeshParts);
    for (uint i = 0u; i < (uint)aMeshData.myParts.size(); ++i)
    {
      MeshPartData& meshPartData = aMeshData.myParts[i];
      ReadVertexInputLayout(meshPartData.myVertexLayoutProperties, aSerializer);
      
      // Vertex data
      {
        uint64 bufferSize;
        aSerializer.Read(bufferSize);

        meshPartData.myVertexData.resize(bufferSize);
        aSerializer.Read(meshPartData.myVertexData.data(), bufferSize);
      }

      // Index data
      {
        uint64 bufferSize;
        aSerializer.Read(bufferSize);

        meshPartData.myIndexData.resize(bufferSize);
        aSerializer.Read(meshPartData.myIndexData.data(), bufferSize);
      }
    }

    return aSerializer.IsGood();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadMaterialInternal(MaterialDesc& aMaterialDesc, BinarySerializer& aSerializer)
  {
    for (glm::float4& param : aMaterialDesc.myParameters)
      aSerializer.Read(param);

    for (String& texture : aMaterialDesc.myTextures)
      aSerializer.Read(texture);

    return aSerializer.IsGood();
  }
//---------------------------------------------------------------------------//
  void BinaryCache::WriteMaterialInternal(const MaterialDesc& aMaterialDesc, BinarySerializer& aSerializer)
  {
    for (const glm::float4& param : aMaterialDesc.myParameters)
      aSerializer.Write(param);

    for (const String& texture : aMaterialDesc.myTextures)
      aSerializer.Write(texture);
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::ReadTextureData(const String& aPath, uint64 aTimeStamp, TextureProperties& someTexPropsOut, TextureData& aTextureDataOut)
  {
    const String cacheFilePath = GetCacheFilePathAbs(aPath.c_str());

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return false;

    BinarySerializer serializer(cacheFilePath.c_str(), READ);
    if (!serializer.IsGood())
      return false;

    uint textureVersion;
    serializer.Read(textureVersion);

    if (textureVersion != kCacheVersion)
      return false;
    
    // Read the texture
    TextureProperties texProps;
    serializer.Read(texProps.path);

    uint dimension;
    serializer.Read(dimension); 
    texProps.myDimension = static_cast<GpuResourceDimension>(dimension);

    serializer.Read(texProps.myWidth);
    serializer.Read(texProps.myHeight);
    serializer.Read(texProps.myDepthOrArraySize);

    uint accessType;
    serializer.Read(accessType);
    texProps.myAccessType = static_cast<CpuMemoryAccessType>(accessType);

    uint format;
    serializer.Read(format);
    texProps.myFormat = static_cast<DataFormat>(format);

    serializer.Read(texProps.myNumMipLevels);

    uint numSavedSubdatas = 0;
    serializer.Read(numSavedSubdatas);
    ASSERT(numSavedSubdatas > 0);

    aTextureDataOut.mySubDatas.resize(numSavedSubdatas);
    
    uint64 totalPixelSize = 0;
    serializer.Read(totalPixelSize);

    aTextureDataOut.myData.resize(totalPixelSize);

    uint8* subPixelDataBuf = aTextureDataOut.myData.data();
    for (uint i = 0u; i < numSavedSubdatas; ++i)
    {
      TextureSubData& subData = aTextureDataOut.mySubDatas[i];
      serializer.Read(subData.myPixelSizeBytes);
      serializer.Read(subData.myRowSizeBytes);
      serializer.Read(subData.mySliceSizeBytes);
      serializer.Read(subData.myTotalSizeBytes);
      serializer.Read(subPixelDataBuf, subData.myTotalSizeBytes);
      subData.myData = subPixelDataBuf;
      subPixelDataBuf += subData.myTotalSizeBytes;
    }

    someTexPropsOut = std::move(texProps);
    return true;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::WriteMesh(const MeshData& aMeshData)
  {
    const String cacheFilePath = GetCacheFilePathAbs(StringUtil::toString(aMeshData.myDesc.myHash).c_str());
    Path::CreateDirectoryTreeForPath(cacheFilePath);

    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open mesh cache file path %s for write", cacheFilePath.c_str());
      return false;
    }
  
    WriteMeshDataInternal(aMeshData, serializer);

    return serializer.IsGood();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp, MeshData& aMeshDataOut)
  {
    uint64 descHash = aDesc.myHash;
    const String cacheFilePath = GetCacheFilePathAbs(StringUtil::toString(descHash).c_str());

    BinarySerializer serializer(cacheFilePath.c_str(), READ);
    if (!serializer.IsGood())
      return nullptr;

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return nullptr;

    uint meshVersion;
    serializer.Read(meshVersion);

    if (meshVersion != kCacheVersion)
      return nullptr;
    
    uint64 descHashInFile;
    serializer.Read(descHashInFile);
    ASSERT(descHashInFile == descHash);

    String name;
    serializer.Read(name);

    aMeshDataOut.myDesc.myHash = descHash;
    aMeshDataOut.myDesc.myName = name;

    uint numMeshParts;
    serializer.Read(numMeshParts);

    aMeshDataOut.myParts.resize(numMeshParts);

    for (uint i = 0u; i < numMeshParts; ++i)
    {
      MeshPartData& partData = aMeshDataOut.myParts[i];
      uint numVertexElements;
      serializer.Read(numVertexElements);

      for (uint iVertexElem = 0u; iVertexElem < numVertexElements; ++iVertexElem)
      {
        VertexInputAttributeDesc elem;
        uint format;
        serializer.Read(format);
        elem.myFormat = static_cast<DataFormat>(format);
        serializer.Read(elem.myBufferIndex);
        uint semantic;
        serializer.Read(semantic);
        elem.mySemantic = static_cast<VertexAttributeSemantic>(semantic);
        serializer.Read(elem.mySemanticIndex);
        partData.myVertexLayoutProperties.myAttributes.Add(elem);
      }

      uint numBufferBindings;
      serializer.Read(numBufferBindings);

      for (uint iBufferBinding = 0u; iBufferBinding < numBufferBindings; ++iBufferBinding)
      {
        VertexBufferBindDesc buffer;
        uint inputRate;
        serializer.Read(inputRate);
        buffer.myInputRate = static_cast<VertexInputRate>(inputRate);
        serializer.Read(buffer.myStride);
        partData.myVertexLayoutProperties.myBufferBindings.Add(buffer);
      }

      // Vertex data
      {
        uint64 totalBufferBytes;
        serializer.Read(totalBufferBytes);
        partData.myVertexData.resize(totalBufferBytes);
        serializer.Read(partData.myVertexData.data(), totalBufferBytes);
      }

      // Index data
      {
        uint64 totalBufferBytes;
        serializer.Read(totalBufferBytes);
        partData.myIndexData.resize(totalBufferBytes);
        serializer.Read(partData.myIndexData.data(), totalBufferBytes);
      }
    }

    return !aMeshDataOut.myParts.empty();
  }
//---------------------------------------------------------------------------//
}
