#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "TextureDesc.h"
#include "RenderCore.h"
#include <fstream>
#include "PathService.h"
#include "StringUtil.h"
#include "RenderCore_Platform.h"
#include "MeshData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  const uint kMeshVersion = 0;
  const uint kTextureVersion = 0;
//---------------------------------------------------------------------------//
  void locWriteString(std::fstream& aStream, const String& aString)
  {
    const char* name_cstr = aString.c_str();
    const uint name_size = static_cast<uint>(aString.size()) + 1u; // size + '/0'
    aStream.write(reinterpret_cast<const char*>(&name_size), sizeof(uint));
    aStream.write(name_cstr, name_size);
  }
  //---------------------------------------------------------------------------//
  String locReadString(std::fstream& aStream)
  {
    uint name_size;
    aStream.read(reinterpret_cast<char*>(&name_size), sizeof(uint));

    const uint kExpectedMaxLength = 64u;
    char buf[kExpectedMaxLength];
    char* name_cstr = buf;

    if (name_size > kExpectedMaxLength)
      name_cstr = new char[name_size];

    aStream.read(name_cstr, name_size);
    String returnStr(name_cstr);

    if (name_size > kExpectedMaxLength)
      delete[] name_cstr;

    return returnStr;
  }
//---------------------------------------------------------------------------//
  String BinaryCache::getCacheFilePathAbs(const String& aPathInResources)
  {
    String basePath = Path::GetUserDataPath();
    return basePath + "ResourceCache/" + aPathInResources + ".bin";
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::WriteTexture(const Texture* aTexture, const TextureSubData& someData)
  {
    const TextureDesc& texDesc = aTexture->GetDescription();
    const uint64 texDescHash = texDesc.GetHash();

    const String cacheFilePath = getCacheFilePathAbs(texDesc.mySourcePath);
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kTextureVersion), sizeof(uint));

    const TextureParams& texParams = aTexture->GetParameters();

    // Write the hash first the be able to "peek ahead" without reading in the whole stuff later on
    archive.write(reinterpret_cast<const char*>(&texDescHash), sizeof(texDescHash));

    // Write the desc
    locWriteString(archive, texDesc.mySourcePath);
    archive.write((const char*)&texDesc.myIsExternalTexture, sizeof(texDesc.myIsExternalTexture));
    archive.write((const char*)&texDesc.myInternalRefIndex, sizeof(texDesc.myInternalRefIndex));
    
    // Write the texture
    archive.write(reinterpret_cast<const char*>(&texParams.myWidth), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.myHeight), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.myDepthOrArraySize), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.myAccessType), sizeof(uint));
    const uint format = static_cast<uint>(texParams.eFormat);
    archive.write(reinterpret_cast<const char*>(&format), sizeof(uint));
    archive.write(reinterpret_cast<const char*>(&texParams.myNumMipLevels), sizeof(uint8));
    
    archive.write(reinterpret_cast<const char*>(&someData.myPixelSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myRowSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.mySliceSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myTotalSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(someData.myData), someData.myTotalSizeBytes);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  SharedPtr<Texture> BinaryCache::ReadTexture(const TextureDesc& aDesc, uint64 aTimeStamp)
  {
    uint64 descHash = aDesc.GetHash();
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(descHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return nullptr;

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return nullptr;

    uint textureVersion;
    archive.read((char*)&textureVersion, sizeof(uint));

    if (textureVersion != kTextureVersion)
      return nullptr;

    TextureDesc texDesc;

    // Read the desc
    texDesc.mySourcePath = locReadString(archive);
    archive.read((char*)&texDesc.myIsExternalTexture, sizeof(texDesc.myIsExternalTexture));
    archive.read((char*)&texDesc.myInternalRefIndex, sizeof(texDesc.myInternalRefIndex));
    
    // Read the texture
    TextureParams texParams;
    texParams.myIsExternalTexture = texDesc.myIsExternalTexture;
    texParams.path = texDesc.mySourcePath;
    texParams.myInternalRefIndex = texDesc.myInternalRefIndex;
    archive.read((char*)&texParams.myWidth, sizeof(uint16));
    archive.read((char*)&texParams.myHeight, sizeof(uint16));
    archive.read((char*)&texParams.myDepthOrArraySize, sizeof(uint16));
    archive.read((char*)&texParams.myAccessType, sizeof(uint));
    uint format = 0;
    archive.read((char*)&format, sizeof(uint));
    texParams.eFormat = static_cast<DataFormat>(format);
    archive.read((char*)&texParams.myNumMipLevels, sizeof(uint8));

    TextureSubData texData;
    archive.read((char*)(&texData.myPixelSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.myRowSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.mySliceSizeBytes), sizeof(uint64));
    archive.write((char*)(&texData.myTotalSizeBytes), sizeof(uint64));
    texData.myData = static_cast<uint8*>(FANCY_ALLOCATE(texData.myTotalSizeBytes, MemoryCategory::TEXTURES));
    archive.read((char*)&texData.myData, texData.myTotalSizeBytes);

    SharedPtr<Texture> newTex(RenderCore::GetPlatform()->CreateTexture());
    newTex->Create(texParams, &texData, 1u);
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
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kMeshVersion), sizeof(uint));

    archive.write(reinterpret_cast<const char*>(&aMesh->myVertexAndIndexHash), sizeof(aMesh->myVertexAndIndexHash));

    const DynamicArray<SharedPtr<GeometryData>>& vGeoData = aMesh->myGeometryDatas;
    const uint numGeoDatas = static_cast<uint>(vGeoData.size());

    ASSERT(numGeoDatas == aNumMeshDatas);

    archive.write(reinterpret_cast<const char*>(&numGeoDatas), sizeof(uint));
    
    for (uint i = 0u; i < vGeoData.size(); ++i)
    {
      const GeometryData* geoData = vGeoData[i].get();

      // Vertex-Layout begin
      const GeometryVertexLayout& vertexLayout = geoData->getGeometryVertexLayout();
      const DynamicArray<GeometryVertexElement>& vVertexElements = vertexLayout.myElements;
      const uint numVertexElements = vVertexElements.size();
      archive.write(reinterpret_cast<const char*>(&numVertexElements), sizeof(uint));

      for (uint iVertexElem = 0u; iVertexElem < vVertexElements.size(); ++iVertexElem)
      {
        const GeometryVertexElement& vertexElement = vVertexElements[iVertexElem];
        const uint semantics = static_cast<uint>(vertexElement.eSemantics);
        archive.write(reinterpret_cast<const char*>(&semantics), sizeof(uint));
        archive.write(reinterpret_cast<const char*>(&vertexElement.u32OffsetBytes), sizeof(uint));
        archive.write(reinterpret_cast<const char*>(&vertexElement.u32SizeBytes), sizeof(uint));
        const uint format = static_cast<uint>(vertexElement.eFormat);
        archive.write(reinterpret_cast<const char*>(&format), sizeof(uint));
      }
      const uint stride = vertexLayout.myStride;
      archive.write(reinterpret_cast<const char*>(&stride), sizeof(uint));
      // Vertex-Layout end

      // Vertex data
      {
        const GpuBuffer* buffer = geoData->getVertexBuffer();
        const GpuBufferCreationParams& bufferParams = buffer->GetParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(GpuBufferCreationParams));
        const uint buffersize = buffer->GetSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint));
        archive.write(reinterpret_cast<const char*>(someMeshDatas[i].myVertexData.data()), DYN_ARRAY_BYTESIZE(someMeshDatas[i].myVertexData));
      }

      // Index data
      {
        const GpuBuffer* buffer = geoData->getIndexBuffer();
        const GpuBufferCreationParams& bufferParams = buffer->GetParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(GpuBufferCreationParams));
        const uint buffersize = buffer->GetSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint));
        archive.write(reinterpret_cast<const char*>(someMeshDatas[i].myIndexData.data()), DYN_ARRAY_BYTESIZE(someMeshDatas[i].myIndexData));
      }
    }

    return archive.good();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> BinaryCache::ReadMesh(const MeshDesc& aDesc, uint64 aTimeStamp)
  {
    uint64 descHash = aDesc.GetHash();
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(descHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return nullptr;

    if (Path::GetFileWriteTime(cacheFilePath) < aTimeStamp)
      return nullptr;

    uint meshVersion;
    archive.read(reinterpret_cast<char*>(&meshVersion), sizeof(uint));

    uint64 vertexIndexHash;
    archive.read(reinterpret_cast<char*>(&vertexIndexHash), sizeof(vertexIndexHash));

    if (meshVersion != kMeshVersion)
      return nullptr;

    uint numGeometryDatas;
    archive.read(reinterpret_cast<char*>(&numGeometryDatas), sizeof(uint));

    DynamicArray<SharedPtr<GeometryData>> vGeoDatas;
    vGeoDatas.resize(numGeometryDatas);

    for (uint i = 0u; i < vGeoDatas.size(); ++i)
    {
      SharedPtr<GeometryData> geoData(FANCY_NEW(GeometryData, MemoryCategory::Geometry));
      vGeoDatas[i] = geoData;

      GeometryVertexLayout vertexLayout;
      uint numVertexElements;
      archive.read(reinterpret_cast<char*>(&numVertexElements), sizeof(uint));

      for (uint iVertexElem = 0u; iVertexElem < numVertexElements; ++iVertexElem)
      {
        GeometryVertexElement elem;
        uint semantics;
        archive.read(reinterpret_cast<char*>(&semantics), sizeof(uint));
        elem.eSemantics = static_cast<VertexSemantics>(semantics);
        archive.read(reinterpret_cast<char*>(&elem.u32OffsetBytes), sizeof(uint));
        archive.read(reinterpret_cast<char*>(&elem.u32SizeBytes), sizeof(uint));
        uint format;
        archive.read(reinterpret_cast<char*>(&format), sizeof(uint));
        elem.eFormat = static_cast<DataFormat>(format);
        vertexLayout.addVertexElement(elem);
      }
      uint strideBytes;
      archive.read(reinterpret_cast<char*>(&strideBytes), sizeof(uint));

      geoData->setVertexLayout(vertexLayout);

      // Vertex data
      {
        GpuBufferCreationParams bufferParams;
        archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(GpuBufferCreationParams));
        uint totalBufferBytes;
        archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint));

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        archive.read((char*)(bufferData), totalBufferBytes);

        SharedPtr<GpuBuffer> buffer = RenderCore::CreateBuffer(bufferParams, bufferData);
        geoData->setVertexBuffer(buffer);

        FANCY_FREE(bufferData, MemoryCategory::Geometry);
      }

      // Index data
      {
        GpuBufferCreationParams bufferParams;
        archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(GpuBufferCreationParams));
        uint totalBufferBytes;
        archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint));

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        archive.read(static_cast<char*>(bufferData), totalBufferBytes);

        SharedPtr<GpuBuffer> buffer = RenderCore::CreateBuffer(bufferParams, bufferData);
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
