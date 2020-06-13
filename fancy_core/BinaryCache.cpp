#include "fancy_core_precompile.h"
#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "RenderCore.h"
#include "PathService.h"
#include "StringUtil.h"
#include "MeshData.h"
#include "DynamicArray.h"
#include "TextureProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  const uint kMeshVersion = 3;
  const uint kTextureVersion = 4;
//---------------------------------------------------------------------------//
  enum SERIALIZE_MODE
  {
    READ, WRITE
  };

  struct BinarySerializer
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
  void BinarySerializer::Write(const uint8* someData, uint64 aDataSize)
  {
    ASSERT(myMode == WRITE);
    myStream.write((const char*) someData, aDataSize);
  }
//---------------------------------------------------------------------------//
  void BinarySerializer::Read(uint8* someData, uint64 aDataSize)
  {
    ASSERT(myMode == READ);
    myStream.read((char*)someData, aDataSize);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinarySerializer::Write(const T& aVal)
  {
    ASSERT(myMode == WRITE);
    Write((const uint8*)(&aVal), sizeof(T));
  }
//---------------------------------------------------------------------------//
  template<>
  void BinarySerializer::Write<String>(const String& aVal)
  {
    ASSERT(myMode == WRITE);
    const char* name_cstr = aVal.c_str();
    const uint64 name_size = static_cast<uint>(aVal.size()) + 1u; // size + '/0'
    Write(name_size);
    Write((const uint8*)name_cstr, name_size);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinarySerializer::Read(T& aVal)
  {
    ASSERT(myMode == READ);
    myStream.read((char*) &aVal, sizeof(T));
  }
//---------------------------------------------------------------------------//
  template<>
  void BinarySerializer::Read<String>(String& aVal)
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
  String BinaryCache::getCacheFilePathAbs(const String& aPathInResources)
  {
    return Path::GetUserDataPath() + "ResourceCache/" + aPathInResources + ".bin";
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::WriteTextureData(const TextureProperties& someTexProps, const TextureSubData* someSubDatas, uint aNumSubDatas)
  {
    const String cacheFilePath = getCacheFilePathAbs(someTexProps.path);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open texture cache file path %s for write", cacheFilePath.c_str());
      return false;
    }

    serializer.Write(kTextureVersion);
    serializer.Write(someTexProps.path);
    
    serializer.Write((uint)someTexProps.myDimension);
    serializer.Write(someTexProps.myWidth);
    serializer.Write(someTexProps.myHeight);
    serializer.Write(someTexProps.myDepthOrArraySize);
    serializer.Write((uint)someTexProps.myAccessType);
    serializer.Write((uint)someTexProps.myFormat);
    serializer.Write(someTexProps.myNumMipLevels);

    serializer.Write(aNumSubDatas);

    uint64 totalPixelSize = 0;
    for (uint i = 0u; i < aNumSubDatas; ++i)
      totalPixelSize += someSubDatas[i].myTotalSizeBytes;

    serializer.Write(totalPixelSize);

    for (uint i = 0u; i < aNumSubDatas; ++i)
    {
      const TextureSubData& someData = someSubDatas[i];
      serializer.Write(someData.myPixelSizeBytes);
      serializer.Write(someData.myRowSizeBytes);
      serializer.Write(someData.mySliceSizeBytes);
      serializer.Write(someData.myTotalSizeBytes);
      serializer.Write(reinterpret_cast<const uint8*>(someData.myData), someData.myTotalSizeBytes);
    }

    return serializer.IsGood();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::ReadTextureData(const String& aPath, uint64 aTimeStamp, TextureProperties& someTexPropsOut, TextureData& aTextureDataOut)
  {
    const String cacheFilePath = getCacheFilePathAbs(aPath);

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return false;

    BinarySerializer serializer(cacheFilePath.c_str(), READ);
    if (!serializer.IsGood())
      return false;

    uint textureVersion;
    serializer.Read(textureVersion);

    if (textureVersion != kTextureVersion)
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
  bool BinaryCache::WriteMesh(const Mesh* aMesh, const MeshData* someMeshDatas, uint aNumMeshDatas)
  {
    const MeshDesc& desc = aMesh->myDesc;
    uint64 descHash = desc.GetHash();

    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(descHash));
    Path::CreateDirectoryTreeForPath(cacheFilePath);

    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open mesh cache file path %s for write", cacheFilePath.c_str());
      return false;
    }

    serializer.Write(kMeshVersion);
    serializer.Write(aMesh->myVertexAndIndexHash);

    const DynamicArray<SharedPtr<GeometryData>>& vGeoData = aMesh->myGeometryDatas;
    const uint numGeoDatas = static_cast<uint>(vGeoData.size());

    ASSERT(numGeoDatas == aNumMeshDatas);

    serializer.Write(numGeoDatas);
    
    for (uint i = 0u; i < vGeoData.size(); ++i)
    {
      const GeometryData* geoData = vGeoData[i].get();

      const VertexInputLayoutProperties& vertexLayout = geoData->GetVertexInputLayout()->myProperties;
      const StaticArray<VertexInputAttributeDesc, 16>& vertexAttributes = vertexLayout.myAttributes;
      const uint numVertexElements = vertexAttributes.Size();
      serializer.Write(numVertexElements);

      for (uint iVertexElem = 0u; iVertexElem < vertexAttributes.Size(); ++iVertexElem)
      {
        const VertexInputAttributeDesc& vertexElement = vertexAttributes[iVertexElem];
        serializer.Write((uint) vertexElement.myFormat);
        serializer.Write(vertexElement.myBufferIndex);
        serializer.Write((uint) vertexElement.mySemantic);
        serializer.Write(vertexElement.mySemanticIndex);
      }

      const StaticArray<VertexBufferBindDesc, 16>& vertexBuffers = vertexLayout.myBufferBindings;
      serializer.Write(vertexBuffers.Size());

      for (uint iBuffer = 0u; iBuffer < vertexBuffers.Size(); ++iBuffer)
      {
        const VertexBufferBindDesc& buffer = vertexBuffers[iBuffer];
        serializer.Write((uint)buffer.myInputRate);
        serializer.Write(buffer.myStride);
      }

      // Vertex data
      {
        const GpuBuffer* buffer = geoData->GetVertexBuffer();
        const GpuBufferProperties& bufferParams = buffer->GetProperties();
        serializer.Write(reinterpret_cast<const uint8*>(&bufferParams), sizeof(GpuBufferProperties));
        const uint64 buffersize = buffer->GetByteSize();
        serializer.Write(buffersize);
        serializer.Write(reinterpret_cast<const uint8*>(someMeshDatas[i].myVertexData.data()), DYN_ARRAY_BYTESIZE(someMeshDatas[i].myVertexData));
      }

      // Index data
      {
        const GpuBuffer* buffer = geoData->GetIndexBuffer();
        const GpuBufferProperties& bufferParams = buffer->GetProperties();
        serializer.Write(reinterpret_cast<const uint8*>(&bufferParams), sizeof(GpuBufferProperties));
        const uint64 buffersize = buffer->GetByteSize();
        serializer.Write(buffersize);
        serializer.Write(reinterpret_cast<const uint8*>(someMeshDatas[i].myIndexData.data()), DYN_ARRAY_BYTESIZE(someMeshDatas[i].myIndexData));
      }
    }

    return serializer.IsGood();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> BinaryCache::ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp)
  {
    uint64 descHash = aDesc.GetHash();
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(descHash));

    BinarySerializer serializer(cacheFilePath.c_str(), READ);
    if (!serializer.IsGood())
      return nullptr;

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return nullptr;

    uint meshVersion;
    serializer.Read(meshVersion);
    
    uint64 vertexIndexHash;
    serializer.Read(vertexIndexHash);

    if (meshVersion != kMeshVersion)
      return nullptr;

    uint numGeometryDatas;
    serializer.Read(numGeometryDatas);

    DynamicArray<SharedPtr<GeometryData>> vGeoDatas;
    vGeoDatas.resize(numGeometryDatas);

    for (uint i = 0u; i < vGeoDatas.size(); ++i)
    {
      SharedPtr<GeometryData> geoData(FANCY_NEW(GeometryData, MemoryCategory::Geometry));
      vGeoDatas[i] = geoData;

      VertexInputLayoutProperties vertexLayout;
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
        vertexLayout.myAttributes.Add(elem);
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
      }

      geoData->SetVertexLayout(RenderCore::CreateVertexInputLayout(vertexLayout));

      // Vertex data
      {
        GpuBufferProperties bufferParams;
        serializer.Read(reinterpret_cast<uint8*>(&bufferParams), sizeof(GpuBufferProperties));
        uint64 totalBufferBytes;
        serializer.Read(totalBufferBytes);

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        serializer.Read((uint8*)(bufferData), totalBufferBytes);

        String name = "VertexBuffer_Mesh_" + aDesc.myUniqueName;
        SharedPtr<GpuBuffer> buffer = RenderCore::CreateBuffer(bufferParams, name.c_str(), bufferData);
        geoData->SetVertexBuffer(buffer);

        FANCY_FREE(bufferData, MemoryCategory::Geometry);
      }

      // Index data
      {
        GpuBufferProperties bufferParams;
        serializer.Read(reinterpret_cast<uint8*>(&bufferParams), sizeof(GpuBufferProperties));
        uint64 totalBufferBytes;
        serializer.Read(totalBufferBytes);

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        serializer.Read(static_cast<uint8*>(bufferData), totalBufferBytes);

        String name = "IndexBuffer_Mesh_" + aDesc.myUniqueName;
        SharedPtr<GpuBuffer> buffer = RenderCore::CreateBuffer(bufferParams, name.c_str(), bufferData);
        geoData->SetIndexBuffer(buffer);

        FANCY_FREE(bufferData, MemoryCategory::Geometry);
      }
    }

    if (vGeoDatas.empty())
      return nullptr;

    SharedPtr<Mesh> newMesh(FANCY_NEW(Mesh, MemoryCategory::GEOMETRY));
    newMesh->myVertexAndIndexHash = vertexIndexHash;
    newMesh->myGeometryDatas = vGeoDatas;
    return newMesh;
  }
//---------------------------------------------------------------------------//
}
