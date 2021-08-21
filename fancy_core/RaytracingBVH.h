#pragma once

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct RaytracingBVHProps
  {
    RaytracingBVHType myType;
    uint myFlags; // RaytracingBVHFlags
  };
//---------------------------------------------------------------------------//
  struct RaytracingBVHGeometry
  {
    const GpuBuffer* myVertexBuffer = nullptr;
    uint64 myVertexBufferOffset = 0;
    const GpuBuffer* myIndexBuffer = nullptr;
    uint64 myIndexBufferOffset = 0;
    const GpuBuffer* myTransformBuffer = 0;
    uint64 myTransformBufferOffset = 0;
    DataFormat myVertexFormat = DataFormat::RGB_32F;
    DataFormat myIndexFormat = DataFormat::R_32UI;
    uint myNumVertices = 0;
    uint myNumIndices = 0;
    uint myFlags = 0; // RaytracingBVHGeometryFlags
    RaytracingBVHGeometryType myType = RaytracingBVHGeometryType::TRIANGLES;
  };
//---------------------------------------------------------------------------//
  class RaytracingBVH
  {
  public:
    RaytracingBVH(const RaytracingBVHProps& someProps);
    virtual ~RaytracingBVH() = default;

    
    
    virtual void Destroy() = 0;

  protected:
    RaytracingBVHProps myProps;
    SharedPtr<GpuBuffer> myBuffer;
  };
//---------------------------------------------------------------------------//
}



