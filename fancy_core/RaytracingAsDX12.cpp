#include "fancy_core_precompile.h"
#include "RaytracingAsDX12.h"

#include "GpuResource.h"
#include "GpuBuffer.h"
#include "GpuResourceDataDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "CommandListDX12.h"

namespace Fancy
{
  namespace Private
  {
    static uint GetBuildGeometryFlags(uint aSomeRaytracingBVHFlags)
    {
      uint flags = 0u;

      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::ALLOW_UPDATE)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::ALLOW_COMPACTION)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::MINIMIZE_MEMORY)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::PREFER_FAST_BUILD)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::PREFER_FAST_TRACE)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

      return flags;
    }
  }

  RaytracingAsDX12::RaytracingAsDX12(const RaytracingAsProps& someProps, const eastl::span<RaytracingAsGeometryInfo>& someGeometries, const char* aName)
    : RaytracingAS(someProps)
  {
    const D3D12_RAYTRACING_GEOMETRY_DESC emptyGeoDesc{};
    eastl::fixed_vector<D3D12_RAYTRACING_GEOMETRY_DESC, 64> geometryDescs(someGeometries.size(), emptyGeoDesc);

    for (uint i = 0u; i < (uint)someGeometries.size(); ++i)
    {
      const RaytracingAsGeometryInfo& geo = someGeometries[i];
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

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS asInputs = {};
    asInputs.Flags = (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS)Private::GetBuildGeometryFlags(someProps.myFlags);
    asInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    asInputs.NumDescs = (uint) geometryDescs.size();
    asInputs.pGeometryDescs = geometryDescs.data();
    asInputs.Type = RenderCore_PlatformDX12::GetRaytracingBVHType(someProps.myType);

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO asPrebuildInfo = {};
    RenderCore::GetPlatformDX12()->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&asInputs, &asPrebuildInfo);
    ASSERT(asPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    GpuBufferProperties bufferProps;
    bufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_BVH_STORAGE;
    bufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    bufferProps.myElementSizeBytes = asPrebuildInfo.ResultDataMaxSizeInBytes;
    bufferProps.myNumElements = 1;
    bufferProps.myIsShaderWritable = true;
    myAccelerationStructureBuffer = RenderCore::CreateBuffer(bufferProps, StaticString<128>("%s_bvh_buffer", aName));
    ASSERT(myAccelerationStructureBuffer != nullptr);

    // Actually build the BVH
    GpuBufferProperties tempBufferProps;
    tempBufferProps.myBindFlags = (uint) GpuBufferBindFlags::RAYTRACING_BVH_BUILD_INPUT;
    tempBufferProps.myIsShaderWritable = true;
    tempBufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    tempBufferProps.myNumElements = 1;
    tempBufferProps.myElementSizeBytes = asPrebuildInfo.ScratchDataSizeInBytes;
    SharedPtr<GpuBuffer> buildTempBuffer = RenderCore::CreateBuffer(tempBufferProps, StaticString<128>("%s_bvh_tempBuffer", aName));

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.Inputs = asInputs;
    asBuildDesc.DestAccelerationStructureData = myAccelerationStructureBuffer->GetDeviceAddress();
    asBuildDesc.ScratchAccelerationStructureData = buildTempBuffer->GetDeviceAddress();

    CommandList* cmdList = RenderCore::BeginCommandList(CommandListType::Graphics);
    ID3D12GraphicsCommandList6* dx12CmdList = static_cast<CommandListDX12*>(cmdList)->GetDX12CommandList();

    dx12CmdList->BuildRaytracingAccelerationStructure(&asBuildDesc, 0, nullptr);
    const GpuResource* res = myAccelerationStructureBuffer.get();
    cmdList->ResourceUAVbarrier(&res, 1);
    RenderCore::ExecuteAndFreeCommandList(cmdList, SyncMode::BLOCKING);
  }

  void RaytracingAsDX12::Destroy()
  {
    myAccelerationStructureBuffer.reset();
  }
}


