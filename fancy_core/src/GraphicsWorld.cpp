#include "GraphicsWorld.h"
#include "MeshDesc.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "GeometryData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GraphicsWorld::GraphicsWorld()
  {

  }
//---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> GraphicsWorld::GetMesh(uint64 aVertexIndexHash)
  {
    auto it = myCachedMeshes.find(aVertexIndexHash);
    if (it != myCachedMeshes.end())
      return it->second;

    SharedPtr<Geometry::Mesh> mesh(FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY));
    if (IO::BinaryCache::read(mesh, aVertexIndexHash, 0u))
    {
      myCachedMeshes.insert(std::make_pair(aVertexIndexHash, mesh));
      return mesh;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> GraphicsWorld::CreateMesh(const Geometry::MeshDesc& aDesc, 
    const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
    const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices)
  {
    SharedPtr<Geometry::Mesh> mesh = GetMesh(aDesc.myVertexAndIndexHash);
    if (mesh != nullptr)
      return mesh;
    
    const uint numSubMeshes = aDesc.myVertexLayouts.size();
    ASSERT(numSubMeshes == someVertexDatas.size() && numSubMeshes == someIndexDatas.size());

    Geometry::GeometryDataList vGeometryDatas;

    for (uint iSubMesh = 0u; iSubMesh < numSubMeshes; ++iSubMesh)
    {
      Geometry::GeometryData* pGeometryData = FANCY_NEW(Geometry::GeometryData, MemoryCategory::GEOMETRY);
      
      const Rendering::GeometryVertexLayout& vertexLayout = aDesc.myVertexLayouts[iSubMesh];

      // Construct the vertex buffer
      void* ptrToVertexData = someVertexDatas[iSubMesh];
      uint numVertices = someNumVertices[iSubMesh];
      
      Rendering::GpuBuffer* vertexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.bIsMultiBuffered = false;
      bufferParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::VERTEX_BUFFER);
      bufferParams.uAccessFlags = static_cast<uint>(Rendering::GpuResourceAccessFlags::NONE);
      bufferParams.uNumElements = numVertices;
      bufferParams.uElementSizeBytes = vertexLayout.getStrideBytes();

      vertexBuffer->create(bufferParams, ptrToVertexData);
      pGeometryData->setVertexLayout(vertexLayout);
      pGeometryData->setVertexBuffer(vertexBuffer);
      
      // Construct the index buffer
      void* ptrToIndexData = someIndexDatas[iSubMesh];
      uint numIndices = someNumIndices[iSubMesh];

      Rendering::GpuBuffer* indexBuffer =
        FANCY_NEW(Rendering::GpuBuffer, MemoryCategory::GEOMETRY);

      Rendering::GpuBufferCreationParams indexBufParams;
      indexBufParams.bIsMultiBuffered = false;
      indexBufParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::INDEX_BUFFER);
      indexBufParams.uAccessFlags = static_cast<uint32>(Rendering::GpuResourceAccessFlags::NONE);
      indexBufParams.uNumElements = numIndices;
      indexBufParams.uElementSizeBytes = sizeof(uint32);

      indexBuffer->create(indexBufParams, ptrToIndexData);
      pGeometryData->setIndexBuffer(indexBuffer);

      vGeometryDatas.push_back(pGeometryData);
    }

    mesh = SharedPtr<Geometry::Mesh>(FANCY_NEW(Geometry::Mesh, MemoryCategory::GEOMETRY));
    mesh->setGeometryDataList(vGeometryDatas);
    mesh->SetVertexIndexHash(aDesc.myVertexAndIndexHash);
    
    myCachedMeshes.insert(std::make_pair(aDesc.myVertexAndIndexHash, mesh));
    IO::BinaryCache::write(mesh, someVertexDatas, someIndexDatas);
    
    return mesh;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
}
