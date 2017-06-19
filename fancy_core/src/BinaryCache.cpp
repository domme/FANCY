#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "TextureDesc.h"
#include "RenderCore.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  const uint32 kMeshVersion = 0;
  const uint32 kTextureVersion = 0;
//---------------------------------------------------------------------------//
  void locWriteString(std::fstream& aStream, const String& aString)
  {
    const char* name_cstr = aString.c_str();
    const uint32 name_size = aString.size() + 1u; // size + '/0'
    aStream.write(reinterpret_cast<const char*>(&name_size), sizeof(uint32));
    aStream.write(name_cstr, name_size);
  }
  //---------------------------------------------------------------------------//
  String locReadString(std::fstream& aStream)
  {
    uint32 name_size;
    aStream.read(reinterpret_cast<char*>(&name_size), sizeof(uint32));

    const uint32 kExpectedMaxLength = 64u;
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
    return PathService::convertToAbsPath(kBinaryCacheRoot) 
      + aPathInResources + kBinaryCacheExtension;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(const SharedPtr<Rendering::Texture>& aTexture, const Rendering::TextureUploadData& someData)
  {
    const Rendering::TextureDesc& texDesc = aTexture->GetDescription();
    const uint64 texDescHash = texDesc.GetHash();

    const String cacheFilePath = getCacheFilePathAbs(texDesc.mySourcePath);
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kTextureVersion), sizeof(uint32));

    const Rendering::TextureParams& texParams = aTexture->GetParameters();

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
    archive.write(reinterpret_cast<const char*>(&texParams.uAccessFlags), sizeof(uint32));
    const uint32 format = static_cast<uint32>(texParams.eFormat);
    archive.write(reinterpret_cast<const char*>(&format), sizeof(uint32));
    archive.write(reinterpret_cast<const char*>(&texParams.u8NumMipLevels), sizeof(uint8));
    
    archive.write(reinterpret_cast<const char*>(&someData.myPixelSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myRowSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.mySliceSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(&someData.myTotalSizeBytes), sizeof(uint64));
    archive.write(reinterpret_cast<const char*>(someData.myData), someData.myTotalSizeBytes);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::read(SharedPtr<Rendering::Texture>* aTexture, uint64 aDescHash, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aDescHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint32 textureVersion;
    archive.read((char*)&textureVersion, sizeof(uint32));

    if (textureVersion != kTextureVersion)
      return false;

    Rendering::TextureDesc texDesc;

    // Read the desc
    texDesc.mySourcePath = locReadString(archive);
    archive.read((char*)&texDesc.myIsExternalTexture, sizeof(texDesc.myIsExternalTexture));
    archive.read((char*)&texDesc.myInternalRefIndex, sizeof(texDesc.myInternalRefIndex));

    SharedPtr<Rendering::Texture> texture = Rendering::RenderCore::GetTexture(texDesc.GetHash());
    if (texture != nullptr)
    {
      *aTexture = texture;
      return true;
    }

    // Read the texture
    Rendering::TextureParams texParams;
    texParams.myIsExternalTexture = texDesc.myIsExternalTexture;
    texParams.path = texDesc.mySourcePath;
    texParams.myInternalRefIndex = texDesc.myInternalRefIndex;
    archive.read((char*)&texParams.u16Width, sizeof(uint16));
    archive.read((char*)&texParams.u16Height, sizeof(uint16));
    archive.read((char*)&texParams.u16Depth, sizeof(uint16));
    archive.read((char*)&texParams.uAccessFlags, sizeof(uint32));
    uint32 format = 0;
    archive.read((char*)&format, sizeof(uint32));
    texParams.eFormat = static_cast<Rendering::DataFormat>(format);
    archive.read((char*)&texParams.u8NumMipLevels, sizeof(uint8));

    Rendering::TextureUploadData texData;
    archive.read((char*)(&texData.myPixelSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.myRowSizeBytes), sizeof(uint64));
    archive.read((char*)(&texData.mySliceSizeBytes), sizeof(uint64));
    archive.write((char*)(&texData.myTotalSizeBytes), sizeof(uint64));
    texData.myData = static_cast<uint8*>(FANCY_ALLOCATE(texData.myTotalSizeBytes, MemoryCategory::TEXTURES));
    archive.read((char*)&texData.myData, texData.myTotalSizeBytes);

    texture = Rendering::RenderCore::CreateTexture(texParams, &texData, 1u);
    FANCY_FREE(texData.myData, MemoryCategory::TEXTURES);

    (*aTexture) = texture;
    
    return false;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(const SharedPtr<Geometry::Mesh>& aMesh, const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aMesh->GetDescription().GetHash()));
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kMeshVersion), sizeof(uint32));
    uint64 hash = aMesh->GetDescription().GetHash();
    archive.write((const char*)&hash, sizeof(hash));

    const Geometry::GeometryDataList& vGeoData = aMesh->getGeometryDataList();
    const uint32 numGeoDatas = vGeoData.size();
    archive.write(reinterpret_cast<const char*>(&numGeoDatas), sizeof(uint32));

    for (uint32 i = 0u; i < vGeoData.size(); ++i)
    {
      const Geometry::GeometryData* geoData = vGeoData[i];

      // Vertex-Layout begin
      const Rendering::GeometryVertexLayout& vertexLayout = geoData->getGeometryVertexLayout();
      const Rendering::VertexElementList& vVertexElements = vertexLayout.getVertexElementList();
      const uint32 numVertexElements = vVertexElements.size();
      archive.write(reinterpret_cast<const char*>(&numVertexElements), sizeof(uint32));

      for (uint32 iVertexElem = 0u; iVertexElem < vVertexElements.size(); ++iVertexElem)
      {
        const Rendering::GeometryVertexElement& vertexElement = vVertexElements[iVertexElem];
        const uint32 semantics = static_cast<uint32>(vertexElement.eSemantics);
        archive.write(reinterpret_cast<const char*>(&semantics), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(&vertexElement.u32OffsetBytes), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(&vertexElement.u32SizeBytes), sizeof(uint32));
        const uint32 format = static_cast<uint32>(vertexElement.eFormat);
        archive.write(reinterpret_cast<const char*>(&format), sizeof(uint32));
      }
      const uint32 stride = vertexLayout.getStrideBytes();
      archive.write(reinterpret_cast<const char*>(&stride), sizeof(uint32));
      // Vertex-Layout end

      // Vertex data
      {
        const Rendering::GpuBuffer* buffer = geoData->getVertexBuffer();
        const Rendering::GpuBufferCreationParams& bufferParams = buffer->GetParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        const uint32 buffersize = buffer->GetSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someVertexDatas[i]), buffer->GetSizeBytes());
      }

      // Index data
      {
        const Rendering::GpuBuffer* buffer = geoData->getIndexBuffer();
        const Rendering::GpuBufferCreationParams& bufferParams = buffer->GetParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        const uint32 buffersize = buffer->GetSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someIndexDatas[i]), buffer->GetSizeBytes());
      }
    }

    return archive.good();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::read(SharedPtr<Geometry::Mesh>& aMesh, uint64 aDescHash, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aDescHash));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint32 meshVersion;
    archive.read(reinterpret_cast<char*>(&meshVersion), sizeof(uint32));

    if (meshVersion != kMeshVersion)
      return false;

    uint64 hash = 0u;
    archive.read((char*)&hash, sizeof(hash));

    uint32 numGeometryDatas;
    archive.read(reinterpret_cast<char*>(&numGeometryDatas), sizeof(uint32));

    Geometry::GeometryDataList vGeoDatas;
    vGeoDatas.resize(numGeometryDatas);

    for (uint32 i = 0u; i < vGeoDatas.size(); ++i)
    {
      Geometry::GeometryData* geoData = FANCY_NEW(Geometry::GeometryData, MemoryCategory::Geometry);
      vGeoDatas[i] = geoData;

      Rendering::GeometryVertexLayout vertexLayout;
      uint32 numVertexElements;
      archive.read(reinterpret_cast<char*>(&numVertexElements), sizeof(uint32));

      for (uint32 iVertexElem = 0u; iVertexElem < numVertexElements; ++iVertexElem)
      {
        Rendering::GeometryVertexElement elem;
        uint32 semantics;
        archive.read(reinterpret_cast<char*>(&semantics), sizeof(uint32));
        elem.eSemantics = static_cast<Rendering::VertexSemantics>(semantics);
        archive.read(reinterpret_cast<char*>(&elem.u32OffsetBytes), sizeof(uint32));
        archive.read(reinterpret_cast<char*>(&elem.u32SizeBytes), sizeof(uint32));
        uint32 format;
        archive.read(reinterpret_cast<char*>(&format), sizeof(uint32));
        elem.eFormat = static_cast<Rendering::DataFormat>(format);
        vertexLayout.addVertexElement(elem);
      }
      uint32 strideBytes;
      archive.read(reinterpret_cast<char*>(&strideBytes), sizeof(uint32));

      geoData->setVertexLayout(vertexLayout);

      // Vertex data
      {
        Rendering::GpuBufferCreationParams bufferParams;
        archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        uint32 totalBufferBytes;
        archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint32));

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        archive.read((char*)(bufferData), totalBufferBytes);

        SharedPtr<Rendering::GpuBuffer> buffer = Rendering::RenderCore::CreateBuffer(bufferParams, bufferData);
        geoData->setVertexBuffer(buffer);

        FANCY_FREE(bufferData, MemoryCategory::Geometry);
      }

      // Index data
      {
        Rendering::GpuBufferCreationParams bufferParams;
        archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        uint32 totalBufferBytes;
        archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint32));

        void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
        archive.read(static_cast<char*>(bufferData), totalBufferBytes);

        SharedPtr<Rendering::GpuBuffer> buffer = Rendering::RenderCore::CreateBuffer(bufferParams, bufferData);
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
} }
