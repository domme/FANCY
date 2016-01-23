#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"
#include "TextureDesc.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  const uint32 kMeshVersion = 0;
  const uint32 kTextureVersion = 0;
//---------------------------------------------------------------------------//
  
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
  bool BinaryCache::write(Rendering::Texture* aTexture, void* someData, uint32 aDataSize)
  {
    ASSERT(someData);
    ASSERT(aDataSize > 0u);

    const String cacheFilePath = getCacheFilePathAbs(aTexture->getPath());
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT_M(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kTextureVersion), sizeof(uint32));

    Rendering::TextureCreationParams texParams = aTexture->getParameters();
    Rendering::TextureDesc texDesc = aTexture->GetDescription();
    uint64 texDescHash = texDesc.GetHash();

    archive.write(reinterpret_cast<const char*>(texDescHash), sizeof(texDescHash));
    archive.write(reinterpret_cast<const char*>(&texParams.u16Width), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.u16Height), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.u16Depth), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&texParams.uAccessFlags), sizeof(uint32));
    const uint32 format = static_cast<uint32>(texParams.eFormat);
    archive.write(reinterpret_cast<const char*>(&format), sizeof(uint32));
    archive.write(reinterpret_cast<const char*>(&texParams.u8NumMipLevels), sizeof(uint8));
    archive.write(reinterpret_cast<const char*>(&aDataSize), sizeof(uint32));
    archive.write(static_cast<const char*>(someData), aDataSize);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::read(Rendering::Texture** aTexture, const Rendering::TextureDesc& aDesc, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(aDesc.mySourcePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    Rendering::Texture* texture = nullptr;

    uint32 textureVersion;
    archive.read((char*)&textureVersion, sizeof(uint32));

    if (textureVersion != kTextureVersion)
      return false;

    uint64 textureHash;
    archive.read((char*)textureHash, sizeof(textureHash));
    
    if (textureHash == aDesc.GetHash())
    {
      Rendering::Texture* textureFromMemCache = Rendering::Texture::Find(textureHash);
      if (nullptr != textureFromMemCache)
      {
        texture = textureFromMemCache;
      }
    }

    if (nullptr == texture)
    {
      texture = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
      Rendering::TextureCreationParams desc;
      desc.myIsExternalTexture = aDesc.myIsExternalTexture;
      desc.path = aDesc.mySourcePath;
      desc.myInternalRefIndex = aDesc.myInternalRefIndex;
      archive.read((char*)&desc.u16Width, sizeof(uint16));
      archive.read((char*)&desc.u16Height, sizeof(uint16));
      archive.read((char*)&desc.u16Depth, sizeof(uint16));
      archive.read((char*)&desc.uAccessFlags, sizeof(uint32));
      uint32 format = 0;
      archive.read((char*)&format, sizeof(uint32));
      desc.eFormat = static_cast<Rendering::DataFormat>(format);
      archive.read((char*)&desc.u8NumMipLevels, sizeof(uint8));
      archive.read((char*)&desc.uPixelDataSizeBytes, sizeof(uint32));
      desc.pPixelData = FANCY_ALLOCATE(desc.uPixelDataSizeBytes, MemoryCategory::TEXTURES);
      archive.read((char*)&desc.pPixelData, desc.uPixelDataSizeBytes);
      texture->create(desc);
      Rendering::Texture::Register(texture);

      FANCY_FREE(desc.pPixelData, MemoryCategory::TEXTURES);
    }

    if ((*aTexture) != nullptr)
      FANCY_DELETE(*aTexture, MemoryCategory::TEXTURES);

    (*aTexture) = texture;
    
    return false;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(Geometry::Mesh* aMesh, void** someVertexDatas, void** someIndexDatas)
  {
    const String cacheFilePath = getCacheFilePathAbs(aMesh->getName());
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT_M(archive.good(), "Failed to open cache file");

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
        const Rendering::GpuBufferCreationParams& bufferParams = buffer->getParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        const uint32 buffersize = buffer->getTotalSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someVertexDatas[i]), buffer->getTotalSizeBytes());
      }

      // Index data
      {
        const Rendering::GpuBuffer* buffer = geoData->getIndexBuffer();
        const Rendering::GpuBufferCreationParams& bufferParams = buffer->getParameters();
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferCreationParams));
        const uint32 buffersize = buffer->getTotalSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someIndexDatas[i]), buffer->getTotalSizeBytes());
      }
    }

    return archive.good();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::read(Geometry::Mesh** aMesh, const Geometry::MeshDesc& aDesc, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(StringUtil::toString(aDesc.GetHash()));
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint32 meshVersion;
    archive.read(reinterpret_cast<char*>(&meshVersion), sizeof(uint32));

    if (meshVersion != kMeshVersion)
      return false;

    Geometry::Mesh* mesh = nullptr;

    uint64 hash;
    archive.read((char*)&hash, sizeof(hash));

    if (hash == aDesc.GetHash())
    {
      Geometry::Mesh* meshFromMemCache = Geometry::Mesh::Find(hash);
      if (nullptr != meshFromMemCache)
      {
        mesh = meshFromMemCache;
      }
    }

    if (nullptr == mesh)
    {
      mesh = FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY);
      mesh->SetVertexIndexHash()
      Geometry::Mesh::registerWithName(mesh);

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
          elem.name = readName(archive);
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
          Rendering::GpuBuffer* buffer = FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::Geometry);
          Rendering::GpuBufferParameters bufferParams;
          buffer->setName(readName(archive));
          archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(Rendering::GpuBufferParameters));
          uint32 totalBufferBytes;
          archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint32));

          void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
          archive.read((char*)(bufferData), totalBufferBytes);
          buffer->create(bufferParams, bufferData);

          FANCY_FREE(bufferData, MemoryCategory::Geometry);
          geoData->setVertexBuffer(buffer);
        }

        // Index data
        {
          Rendering::GpuBuffer* buffer = FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::Geometry);
          Rendering::GpuBufferParameters bufferParams;
          buffer->setName(readName(archive));
          archive.read(reinterpret_cast<char*>(&bufferParams), sizeof(Rendering::GpuBufferParameters));
          uint32 totalBufferBytes;
          archive.read(reinterpret_cast<char*>(&totalBufferBytes), sizeof(uint32));

          void* bufferData = FANCY_ALLOCATE(totalBufferBytes, MemoryCategory::Geometry);
          archive.read(static_cast<char*>(bufferData), totalBufferBytes);
          buffer->create(bufferParams, bufferData);
          FANCY_FREE(bufferData, MemoryCategory::Geometry);

          geoData->setIndexBuffer(buffer);
        }
      }

      mesh->setGeometryDataList(vGeoDatas);
    }

    if ((*aMesh) != nullptr)
    {
      FANCY_DELETE((*aMesh), MemoryCategory::GEOMETRY);
      (*aMesh) = mesh;
    }

    return true;
  }
//---------------------------------------------------------------------------//
} }
