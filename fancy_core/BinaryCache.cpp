#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "RenderCore.h"
#include "PathService.h"
#include "StringUtil.h"
#include "RenderCore_Platform.h"
#include "MeshData.h"

#include <fstream>
#include "GpuProgramProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  const uint kMeshVersion = 0;
  const uint kTextureVersion = 1;
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
  bool BinaryCache::WriteTexture(const Texture* aTexture, const TextureSubData& someData)
  {
    const String cacheFilePath = getCacheFilePathAbs(aTexture->GetProperties().path);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    BinarySerializer serializer(cacheFilePath.c_str(), WRITE);

    if (!serializer.IsGood())
    {
      LOG_WARNING("Failed to open texture cache file path % for write", cacheFilePath.c_str());
      return false;
    }
    
    const TextureProperties& texProps = aTexture->GetProperties();

    serializer.Write(kTextureVersion);
    serializer.Write(texProps.path);
    
    serializer.Write((uint)texProps.myDimension);
    serializer.Write(texProps.myWidth);
    serializer.Write(texProps.myHeight);
    serializer.Write(texProps.myDepthOrArraySize);
    serializer.Write((uint)texProps.myAccessType);
    serializer.Write((uint)texProps.eFormat);
    serializer.Write(texProps.myNumMipLevels);
    
    serializer.Write(someData.myPixelSizeBytes);
    serializer.Write(someData.myRowSizeBytes);
    serializer.Write(someData.mySliceSizeBytes);
    serializer.Write(someData.myTotalSizeBytes);
    serializer.Write(reinterpret_cast<const uint8*>(someData.myData), someData.myTotalSizeBytes);

    return serializer.IsGood();
  }  
//---------------------------------------------------------------------------//  
  SharedPtr<Texture> BinaryCache::ReadTexture(const String& aPath, uint64 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(aPath);

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return nullptr;

    BinarySerializer serializer(cacheFilePath.c_str(), READ);
    if (!serializer.IsGood())
      return nullptr;

    uint textureVersion;
    serializer.Read(textureVersion);

    if (textureVersion != kTextureVersion)
      return nullptr;
    
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
    texProps.myAccessType = static_cast<GpuMemoryAccessType>(accessType);

    uint format;
    serializer.Read(format);
    texProps.eFormat = static_cast<DataFormat>(format);

    serializer.Read(texProps.myNumMipLevels);

    TextureSubData texData;
    serializer.Read(texData.myPixelSizeBytes);
    serializer.Read(texData.myRowSizeBytes);
    serializer.Read(texData.mySliceSizeBytes);
    serializer.Read(texData.myTotalSizeBytes);
    
    texData.myData = static_cast<uint8*>(FANCY_ALLOCATE(texData.myTotalSizeBytes, MemoryCategory::TEXTURES));
    serializer.Read(texData.myData, texData.myTotalSizeBytes);
    
    SharedPtr<Texture> newTex(RenderCore::GetPlatform()->CreateTexture());
    newTex->Create(texProps, texProps.path.c_str(), &texData, 1u);
    FANCY_FREE(texData.myData, MemoryCategory::TEXTURES);
    return newTex;
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
      LOG_WARNING("Failed to open mesh cache file path % for write", cacheFilePath.c_str());
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

      const GeometryVertexLayout& vertexLayout = geoData->getGeometryVertexLayout();
      const DynamicArray<GeometryVertexElement>& vVertexElements = vertexLayout.myElements;
      const uint64 numVertexElements = vVertexElements.size();
      serializer.Write(numVertexElements);

      for (uint iVertexElem = 0u; iVertexElem < vVertexElements.size(); ++iVertexElem)
      {
        const GeometryVertexElement& vertexElement = vVertexElements[iVertexElem];
        serializer.Write(vertexElement.name);
        serializer.Write((uint) vertexElement.eSemantics);
        serializer.Write(vertexElement.u32OffsetBytes);
        serializer.Write(vertexElement.u32SizeBytes);
        serializer.Write((uint)vertexElement.eFormat);
      }

      // Vertex data
      {
        const GpuBuffer* buffer = geoData->getVertexBuffer();
        const GpuBufferProperties& bufferParams = buffer->GetProperties();
        serializer.Write(reinterpret_cast<const uint8*>(&bufferParams), sizeof(GpuBufferProperties));
        const uint64 buffersize = buffer->GetByteSize();
        serializer.Write(buffersize);
        serializer.Write(reinterpret_cast<const uint8*>(someMeshDatas[i].myVertexData.data()), DYN_ARRAY_BYTESIZE(someMeshDatas[i].myVertexData));
      }

      // Index data
      {
        const GpuBuffer* buffer = geoData->getIndexBuffer();
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

      GeometryVertexLayout vertexLayout;
      uint64 numVertexElements;
      serializer.Read(numVertexElements);

      for (uint iVertexElem = 0u; iVertexElem < numVertexElements; ++iVertexElem)
      {
        GeometryVertexElement elem;

        serializer.Read(elem.name);
        uint semantics;
        serializer.Read(semantics);
        elem.eSemantics = static_cast<VertexSemantics>(semantics);
        serializer.Read(elem.u32OffsetBytes);
        serializer.Read(elem.u32SizeBytes);
        uint format;
        serializer.Read(format);
        elem.eFormat = static_cast<DataFormat>(format);

        vertexLayout.addVertexElement(elem);
      }

      geoData->setVertexLayout(vertexLayout);


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
        geoData->setVertexBuffer(buffer);

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
        geoData->setIndexBuffer(buffer);

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
