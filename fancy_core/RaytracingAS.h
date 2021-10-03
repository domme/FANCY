#pragma once

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct RaytracingAsProps
  {
    RaytracingBVHType myType;
    uint myFlags; // RaytracingBVHFlags
  };
//---------------------------------------------------------------------------//
  struct RaytracingAsGeometryInfo
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
  class RaytracingAS
  {
  public:
    RaytracingAS(const RaytracingAsProps& someProps);
    virtual ~RaytracingAS() = default;

    
    
    virtual void Destroy() = 0;

  protected:
    RaytracingAsProps myProps;
    SharedPtr<GpuBuffer> myBuffer;
  };
//---------------------------------------------------------------------------//
}



