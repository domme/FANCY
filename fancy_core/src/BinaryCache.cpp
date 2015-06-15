#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  const uint32 kMeshVersion = 0;
  const uint32 kTextureVersion = 0;
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  const String kBinaryCacheRoot = "Cache/";
  const String kBinaryCacheExtension = ".bin";
//---------------------------------------------------------------------------//
  void writeName(std::fstream& aStream, const ObjectName& anObjectName)
  {
    String nameStr = anObjectName.toString();
    const char* name_cstr = nameStr.c_str();
    const uint32 name_size = nameStr.size() + 1u; // size + '/0'
    aStream.write(reinterpret_cast<const char*>(&name_size), sizeof(uint32));
    aStream.write(name_cstr, name_size);
  }
//---------------------------------------------------------------------------//
  ObjectName readName(std::fstream& aStream)
  {
    uint32 name_size;
    aStream.read(reinterpret_cast<char*>(&name_size), sizeof(uint32));

    const uint32 kExpectedMaxLength = 64u;
    char buf[kExpectedMaxLength];
    char* name_cstr = buf;
    
    if (name_size > kExpectedMaxLength)
      name_cstr = new char[name_size];

    aStream.read(name_cstr, name_size);
    ObjectName returnName = String(name_cstr);

    if (name_size > kExpectedMaxLength)
      delete[] name_cstr;

    return returnName;
  }
//---------------------------------------------------------------------------//
  String BinaryCache::getCacheFilePathAbs(const ObjectName& aName)
  {
    return PathService::convertToAbsPath(kBinaryCacheRoot) 
      + aName.toString() + kBinaryCacheExtension;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(Rendering::Texture* aTexture, void* someData, uint32 aDataSize)
  {
    ASSERT(someData);
    ASSERT(aDataSize > 0u);

    Rendering::TextureDesc textureDesc = aTexture->getParameters();
    textureDesc.pPixelData = someData;
    textureDesc.uPixelDataSizeBytes = aDataSize;

    const String cacheFilePath = getCacheFilePathAbs(aTexture->getPath());
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    ASSERT_M(archive.good(), "Failed to open cache file");

    archive.write(reinterpret_cast<const char*>(&kTextureVersion), sizeof(uint32));

    Rendering::TextureDesc desc = aTexture->getParameters();
    writeName(archive, desc.path);
    archive.write(reinterpret_cast<const char*>(&desc.u16Width), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&desc.u16Height), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&desc.u16Depth), sizeof(uint16));
    archive.write(reinterpret_cast<const char*>(&desc.uAccessFlags), sizeof(uint32));
    const uint32 format = static_cast<uint32>(desc.eFormat);
    archive.write(reinterpret_cast<const char*>(&format), sizeof(uint32));
    archive.write(reinterpret_cast<const char*>(&desc.u8NumMipLevels), sizeof(uint8));
    archive.write(reinterpret_cast<const char*>(&aDataSize), sizeof(uint32));
    archive.write(static_cast<const char*>(someData), aDataSize);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::read(Rendering::Texture** aTexture, const ObjectName& aName, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(aName);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint32 textureVersion;
    archive.read((char*)&textureVersion, sizeof(uint32));

    if (textureVersion != kTextureVersion)
      return false;

    ObjectName textureName = readName(archive);
    if ((*aTexture) != nullptr && (*aTexture)->getPath() == textureName)
      return true;

    if ((*aTexture) != nullptr)
    {
      FANCY_DELETE(*aTexture, MemoryCategory::TEXTURES);
      (*aTexture) = nullptr;
    }

    (*aTexture) = Rendering::Texture::getByName(textureName);
    if ((*aTexture) != nullptr)
      return true;

    Rendering::Texture* texture = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
    texture->setPath(textureName);
    Rendering::Texture::registerWithName(textureName, texture);
    (*aTexture) = texture;

    Rendering::TextureDesc desc;
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
    FANCY_FREE(desc.pPixelData, MemoryCategory::TEXTURES);

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
    writeName(archive, aMesh->getName());

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
        writeName(archive, vertexElement.name);
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
        const Rendering::GpuBufferParameters& bufferParams = buffer->getParameters();
        writeName(archive, buffer->getName());
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferParameters));
        const uint32 buffersize = buffer->getTotalSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someVertexDatas[i]), buffer->getTotalSizeBytes());
      }

      // Index data
      {
        const Rendering::GpuBuffer* buffer = geoData->getIndexBuffer();
        const Rendering::GpuBufferParameters& bufferParams = buffer->getParameters();
        writeName(archive, buffer->getName());
        archive.write(reinterpret_cast<const char*>(&bufferParams), sizeof(Rendering::GpuBufferParameters));
        const uint32 buffersize = buffer->getTotalSizeBytes();
        archive.write(reinterpret_cast<const char*>(&buffersize), sizeof(uint32));
        archive.write(reinterpret_cast<const char*>(someIndexDatas[i]), buffer->getTotalSizeBytes());
      }
    }

    return archive.good();
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::read(Geometry::Mesh** aMesh, const ObjectName& aName, uint32 aTimeStamp)
  {
    const String cacheFilePath = getCacheFilePathAbs(aName);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

    if (!archive.good())
      return false;

    uint32 meshVersion;
    archive.read(reinterpret_cast<char*>(&meshVersion), sizeof(uint32));

    if (meshVersion != kMeshVersion)
      return false;
    
    if ((*aMesh) != nullptr)
    {
      FANCY_DELETE((*aMesh), MemoryCategory::GEOMETRY);
      (*aMesh) = nullptr;
    }

    ObjectName meshName = readName(archive);
    (*aMesh) = Geometry::Mesh::getByName(meshName);
    if ((*aMesh) != nullptr)
      return true;

    (*aMesh) = FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY);
    Geometry::Mesh* mesh = (*aMesh);
    mesh->setName(meshName);
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

    return true;
  }
//---------------------------------------------------------------------------//
} }