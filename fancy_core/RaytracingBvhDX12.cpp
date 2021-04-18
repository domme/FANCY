#include "fancy_core_precompile.h"
#include "RaytracingBvhDX12.h"


#include "GpuBuffer.h"
#include "GpuResourceDataDX12.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy
{
  RaytracingBvhDX12::RaytracingBvhDX12(const RaytracingBVHProps& someProps, const eastl::span<RaytracingBVHGeometry>& someGeometries, const char* aName)
    : RaytracingBVH(someProps)
  {
    const D3D12_RAYTRACING_GEOMETRY_DESC emptyGeoDesc{};
    eastl::fixed_vector<D3D12_RAYTRACING_GEOMETRY_DESC, 64> geometryDescs(someGeometries.size(), emptyGeoDesc);

    for (uint i = 0u; i < (uint) someGeometries.size(); ++i)
    {
      const RaytracingBVHGeometry& geo = someGeometries[i];
      const uint vertexStride = DataFormatInfo::GetFormatInfo(geo.myVertexFormat).mySizeBytes;
      const uint indexStride = DataFormatInfo::GetFormatInfo(geo.myIndexFormat).mySizeBytes;

      D3D12_RAYTRACING_GEOMETRY_DESC& geoDesc = geometryDescs[i];

      if (geo.myFlags & (uint)RaytracingBVHGeometryFlags::OPAQUE_GEOMETRY)
        geoDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
      if (geo.myFlags & (uint)RaytracingBVHGeometryFlags::NO_DUPLICATE_ANYHIT_INVOCATION)
        geoDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

      geoDesc.Type = RenderCore_PlatformDX12::GetRaytracingBVHGeometryType(geo.myType);

      if (geoDesc.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
      {
        geoDesc.Triangles.VertexFormat = RenderCore_PlatformDX12::ResolveFormat(geo.myVertexFormat);
        geoDesc.Triangles.VertexBuffer.StartAddress = geo.myVertexBuffer->GetDeviceAddress() + geo.myVertexBufferOffset;
        geoDesc.Triangles.VertexBuffer.StrideInBytes = vertexStride;
        geoDesc.Triangles.VertexCount = geo.myNumVertices;
        geoDesc.Triangles.IndexFormat = RenderCore_PlatformDX12::ResolveFormat(geo.myIndexFormat);
        geoDesc.Triangles.IndexBuffer = geo.myIndexBuffer->GetDeviceAddress() + geo.myIndexBufferOffset;
        geoDesc.Triangles.IndexCount = geo.myNumIndices;
        geoDesc.Triangles.Transform3x4 = geo.myTransformBuffer ? geo.myTransformBuffer->GetDeviceAddress() + geo.myTransformBufferOffset : 0;
      }
      else
      {
        ASSERT(false, "Not implemented yet");
      }
    }



  }

  RaytracingBvhDX12::~RaytracingBvhDX12()
  {
  }

  void RaytracingBvhDX12::Destroy()
  {
  }
}


