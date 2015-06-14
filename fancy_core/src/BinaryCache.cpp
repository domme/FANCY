#include "BinaryCache.h"
#include "GeometryData.h"
#include "Mesh.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  const uint32 kMeshVersion = 0;
  const uint32 kTextureVersion = 0;
//---------------------------------------------------------------------------//
  struct TextureHeader
  {
    uint32 myWidth;
    uint32 myHeight;
    uint32 myDepth;
    uint32 myFormat;
    uint32 myAccessFlags;
    uint32 myPixelDataSizeBytes;
    uint32 myNumMipmapLevels;
  };
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

    Rendering::TextureDesc desc = aTexture->getParameters();

    TextureHeader header;
    header.myWidth = desc.u16Width;
    header.myHeight = desc.u16Height;
    header.myDepth = desc.u16Depth;
    header.myAccessFlags = desc.uAccessFlags;
    header.myFormat = static_cast<uint32>(desc.eFormat);
    header.myNumMipmapLevels = desc.u8NumMipLevels;
    header.myPixelDataSizeBytes = aDataSize;
    archive.write(reinterpret_cast<const char*>(&header), sizeof(TextureHeader));
    archive.write(static_cast<const char*>(someData), aDataSize);

    return archive.good();
  }  
//---------------------------------------------------------------------------//  
  bool BinaryCache::read(Rendering::Texture** aTexture, const ObjectName& aName, uint32 aTimeStamp)
  {
    /*TextureHeader header;
    mySceneGraphStr->read((char*)&header, sizeof(TextureHeader));

    aTextureDesc->path = header.myPath.toString();
    aTextureDesc->u16Width = header.myWidth;
    aTextureDesc->u16Height = header.myHeight;
    aTextureDesc->u16Depth = header.myDepth;
    aTextureDesc->eFormat = static_cast<Rendering::DataFormat>(header.myFormat);
    aTextureDesc->u8NumMipLevels = header.myNumMipmapLevels;
    aTextureDesc->uAccessFlags = header.myAccessFlags;
    aTextureDesc->uPixelDataSizeBytes = header.myPixelDataSizeBytes;

    aTextureDesc->pPixelData = FANCY_ALLOCATE(header.myPixelDataSizeBytes, MemoryCategory::TEXTURES);
    ASSERT(aTextureDesc->pPixelData);

    mySceneGraphStr->read((char*)aTextureDesc->pPixelData, header.myPixelDataSizeBytes);*/

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
      FANCY_DELETE((*aMesh), MemoryCategory::GEOMETRY);

    ObjectName meshName = readName(archive);
    /*(*aMesh) = Geometry::Mesh::getByName(meshName);
    if ((*aMesh) != nullptr)
      return true;*/

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
        archive.read((char*)(bufferData), totalBufferBytes);
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