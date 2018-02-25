#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "TextureDesc.h"
#include "RenderCore.h"
#include <fstream>
#include "PathService.h"
#include "StringUtil.h"

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
  const String kBinaryCacheRoot = "Cache/";
  const String kBinaryCacheExtension = ".bin";
//---------------------------------------------------------------------------//
  String BinaryCache::getCacheFilePathAbs(const String& aPathInResources)
  {
    const String resourceName = kBinaryCacheRoot + aPathInResources + kBinaryCacheExtension;
    return Path::GetAbsolutePath(resourceName);
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(const SharedPtr<Texture>& aTexture, const TextureUploadData& someData)
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
    archive.write(reinterpret_cast<const char*>(&texParams.u16Width), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.u16Height), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.u16Depth), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.uAccessFlags), sizeof(uint));
    const uint format = static_cast<uint>(texParams.eFormat);
    archive.write(reinterpret_cast<const char*>(&format), sizeof(uint));
    archive.write(reinterpret_cast<const char*>(&texParams.u8NumMipLevels), sizeof(uint8));
    
    archive.write(reinterpret_cast<const char*>(&someData.myPixelSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myRowSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.mySliceSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myTotalSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(someData.myData), someData.myTotalSizeBytes);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::read(SharedPtr<Texture>* aTexture, uint64 aDescHash, uint aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aDescHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint textureVersion;
    archive.read((char*)&textureVersion, sizeof(uint));

    if (textureVersion != kTextureVersion)
      return false;

    TextureDesc texDesc;

    // Read the desc
    texDesc.mySourcePath = locReadString(archive);
    archive.read((char*)&texDesc.myIsExternalTexture, sizeof(texDesc.myIsExternalTexture));
    archive.read((char*)&texDesc.myInternalRefIndex, sizeof(texDesc.myInternalRefIndex));

    SharedPtr<Texture> texture = RenderCore::GetTexture(texDesc.GetHash());
    if (texture != nullptr)
    {
      *aTexture = texture;
      return true;
    }

    // Read the texture
    TextureParams texParams;
    texParams.myIsExternalTexture = texDesc.myIsExternalTexture;
    texParams.path = texDesc.mySourcePath;
    texParams.myInternalRefIndex = texDesc.myInternalRefIndex;
    archive.read((char*)&texParams.u16Width, sizeof(uint16));
    archive.read((char*)&texParams.u16Height, sizeof(uint16));
    archive.read((char*)&texParams.u16Depth, sizeof(uint16));
    archive.read((char*)&texParams.uAccessFlags, sizeof(uint));
    uint format = 0;
    archive.read((char*)&format, sizeof(uint));
    texParams.eFormat = static_cast<DataFormat>(format);
    archive.read((char*)&texParams.u8NumMipLevels, sizeof(uint8));

    TextureUploadData texData;
    archive.read((char*)(&texData.myPixelSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.myRowSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.mySliceSizeBytes), sizeof(uint64));
    archive.write((char*)(&texData.myTotalSizeBytes), sizeof(uint64));
    texData.myData = static_cast<uint8*>(FANCY_ALLOCATE(texData.myTotalSizeBytes, MemoryCategory::TEXTURES));
    archive.read((char*)&texData.myData, texData.myTotalSizeBytes);

    texture = RenderCore::CreateTexture(texParams, &texData, 1u);
    FANCY_FREE(texData.myData, MemoryCategory::TEXTURES);

    (*aTexture) = texture;
    
    return false;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(const SharedPtr<Mesh>& aMesh, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aMesh->GetGeometryHash()));
    Path::CreateDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kMeshVersion), sizeof(uint));
    uint64 hash = aMesh->GetGeometryHash();
    archive.write((const char*)&hash, sizeof(hash));

    const DynamicArray<GeometryData*>& vGeoData = aMesh->getGeometryDataList();
    const uint numGeoDatas = vGeoData.size();
    archive.write(reinterpret_cast<const char*>(&numGeoDatas), sizeof(uint));

    for (uint i = 0u; i < vGeoData.size(); ++i)
    {
      const GeometryData* geoData = vGeoData[i];

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
        archive.write(reinterpret_cast<const char*>(someVertexDatas[i]), buffer->GetSizeBytes());
      }

      // Index data
      {
        const GpuBuffer* buffer = geoData->getIndexBuffer();
        const GpuBufferCreationParams& bufferParams = buffer->GetParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(GpuBufferCreationParams));
        const uint buffersize = buffer->GetSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint));
        archive.write(reinterpret_cast<const char*>(someIndexDatas[i]), buffer->GetSizeBytes());
      }
    }

    return archive.good();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::read(SharedPtr<Mesh>& aMesh, uint64 aDescHash, uint aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aDescHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint meshVersion;
    archive.read(reinterpret_cast<char*>(&meshVersion), sizeof(uint));

    if (meshVersion != kMeshVersion)
      return false;

    uint64 hash = 0u;
    archive.read((char*)&hash, sizeof(hash));

    uint numGeometryDatas;
    archive.read(reinterpret_cast<char*>(&numGeometryDatas), sizeof(uint));

    DynamicArray<GeometryData*> vGeoDatas;
    vGeoDatas.resize(numGeometryDatas);

    for (uint i = 0u; i < vGeoDatas.size(); ++i)
    {
      GeometryData* geoData = FANCY_NEW(GeometryData, MemoryCategory::Geometry);
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
      return false;

    aMesh->SetVertexIndexHash(aDescHash);
    aMesh->setGeometryDataList(vGeoDatas);
    return true;
  }
//---------------------------------------------------------------------------//
}
