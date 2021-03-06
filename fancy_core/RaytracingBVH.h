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
    RaytracingBVHGeometryType myType;
    uint myFlags; // RaytracingBVHGeometryFlags
    DataFormat myVertexFormat;
    const GpuBuffer* myVertexBuffer;
    DataFormat myIndexFormat;
    const GpuBuffer* myIndexBuffer;
    const GpuBuffer* myTransformBuffer;
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



