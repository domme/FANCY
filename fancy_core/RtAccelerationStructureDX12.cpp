#include "fancy_core_precompile.h"
#include "RtAccelerationStructureDX12.h"

#include "GpuResource.h"
#include "GpuBuffer.h"
#include "GpuResourceDataDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "CommandListDX12.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Private
  {
    static uint GetAccelerationStructureFlags(uint someFlags)
    {
      uint flags = 0u;
      if (someFlags & (uint)RtAccelerationStructureFlags::ALLOW_UPDATE)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
      if (someFlags & (uint)RtAccelerationStructureFlags::ALLOW_COMPACTION)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
      if (someFlags & (uint)RtAccelerationStructureFlags::MINIMIZE_MEMORY)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
      if (someFlags & (uint)RtAccelerationStructureFlags::PREFER_FAST_BUILD)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
      if (someFlags & (uint)RtAccelerationStructureFlags::PREFER_FAST_TRACE)
        flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

      return flags;
    }

    static D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE GetBufferData(const RtBufferData& aBufferData, CommandList* aCommandList)
    {
      if (aBufferData.myType == RT_BUFFER_DATA_TYPE_NONE)
        return { 0, 0 };

      if (aBufferData.myType == RT_BUFFER_DATA_TYPE_GPU_BUFFER)
        return { aBufferData.myBuffer.myBuffer->GetDeviceAddress() + aBufferData.myBuffer.myOffsetBytes, aBufferData.myBuffer.mySizeBytes };

      uint64 offset;
      const GpuBuffer* buffer = aCommandList->GetBuffer(offset, GpuBufferUsage::STAGING_UPLOAD, aBufferData.myCpuData.myData, aBufferData.myCpuData.myDataSize);
      ASSERT(buffer != nullptr);
      return { buffer->GetDeviceAddress() + offset, aBufferData.myCpuData.myDataSize };
    }

    static void GetBLASGeometryDescs(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, CommandList* cmdList, eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs)
    {
      geometryDescs.reserve(aNumGeometries);

      for (uint i = 0u; i < aNumGeometries; ++i)
      {
        const RtAccelerationStructureGeometryData& geoInfo = someGeometries[i];
        const uint vertexStride = DataFormatInfo::GetFormatInfo(geoInfo.myVertexFormat).mySizeBytes;
        const uint indexStride = DataFormatInfo::GetFormatInfo(geoInfo.myIndexFormat).mySizeBytes;

        D3D12_RAYTRACING_GEOMETRY_DESC& geoDescDx12 = geometryDescs.push_back();
        memset(&geoDescDx12, 0, sizeof(geoDescDx12));

        if (geoInfo.myFlags & (uint)RtAccelerationStructureGeometryFlags::OPAQUE_GEOMETRY)
          geoDescDx12.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        if (geoInfo.myFlags & (uint)RtAccelerationStructureGeometryFlags::NO_DUPLICATE_ANYHIT_INVOCATION)
          geoDescDx12.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

        geoDescDx12.Type = RenderCore_PlatformDX12::GetRaytracingBVHGeometryType(geoInfo.myType);
        if (geoDescDx12.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
        {
          D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE vertexBuffer = Private::GetBufferData(geoInfo.myVertexData, cmdList);
          D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE indexBuffer = Private::GetBufferData(geoInfo.myIndexData, cmdList);
          D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE transformBuffer = Private::GetBufferData(geoInfo.myTransformData, cmdList);

          geoDescDx12.Triangles.VertexFormat = RenderCore_PlatformDX12::ResolveFormat(geoInfo.myVertexFormat);
          geoDescDx12.Triangles.VertexBuffer.StartAddress = vertexBuffer.StartAddress;
          geoDescDx12.Triangles.VertexBuffer.StrideInBytes = vertexBuffer.StrideInBytes;
          geoDescDx12.Triangles.VertexCount = geoInfo.myNumVertices;
          geoDescDx12.Triangles.IndexFormat = RenderCore_PlatformDX12::ResolveFormat(geoInfo.myIndexFormat);
          geoDescDx12.Triangles.IndexBuffer = indexBuffer.StartAddress;
          geoDescDx12.Triangles.IndexCount = geoInfo.myNumIndices;
          geoDescDx12.Triangles.Transform3x4 = transformBuffer.StartAddress;
        }
        else
        {
          D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE aabbBuffer = Private::GetBufferData(geoInfo.myProcedural_AABBData, cmdList);
          geoDescDx12.AABBs.AABBs = aabbBuffer;
          geoDescDx12.AABBs.AABBCount = geoInfo.myProcedural_NumAABBs;
        }
      }
    }

    static void GetTLASInstanceDescs(const RtAccelerationStructureInstanceData* someInstanceDatas, uint aNumInstances, eastl::vector<D3D12_RAYTRACING_INSTANCE_DESC>& someInstanceDescsOut)
    {
      someInstanceDescsOut.reserve(aNumInstances);

      for (uint i = 0u; i < aNumInstances; ++i)
      {
        const RtAccelerationStructureInstanceData& instanceData = someInstanceDatas[i];
        D3D12_RAYTRACING_INSTANCE_DESC& instanceDescDx12 = someInstanceDescsOut.push_back();
        memset(&instanceDescDx12, 0, sizeof(instanceDescDx12));

        instanceDescDx12.Flags = 0;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
          instanceDescDx12.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
          instanceDescDx12.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_FORCE_OPAQUE)
          instanceDescDx12.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_FORCE_NONE_OPAQUE)
          instanceDescDx12.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE;

        instanceDescDx12.InstanceContributionToHitGroupIndex = instanceData.mySbtHitGroupOffset;
        instanceDescDx12.InstanceID = instanceData.myInstanceId;
        instanceDescDx12.InstanceMask = instanceData.myInstanceMask;

        for (int row = 0; row < 3; ++row)
          for (int col = 0; col < 4; ++col)
            instanceDescDx12.Transform[row][col] = instanceData.myTransform[row][col];

        instanceDescDx12.AccelerationStructure = instanceData.myInstanceBLAS->GetBuffer()->GetDeviceAddress();
      }
    }
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureDX12::RtAccelerationStructureDX12(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint someFlags /*= 0*/, const char* aName /*= nullptr*/)
    : RtAccelerationStructure(RtAccelerationStructureType::BOTTOM_LEVEL, aName)
  {
    CommandList* cmdList = RenderCore::BeginCommandList(CommandListType::Graphics);
    eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
    Private::GetBLASGeometryDescs(someGeometries, aNumGeometries, cmdList, geometryDescs);
    BuildInternal(&geometryDescs, nullptr, someFlags, aName, cmdList);
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureDX12::RtAccelerationStructureDX12(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags, const char* aName)
    : RtAccelerationStructure(RtAccelerationStructureType::TOP_LEVEL, aName)
  {
    eastl::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
    Private::GetTLASInstanceDescs(someInstances, aNumInstances, instanceDescs);

    CommandList* cmdList = RenderCore::BeginCommandList(CommandListType::Graphics);
    BuildInternal(nullptr, &instanceDescs, someFlags, aName, cmdList);
  }
//---------------------------------------------------------------------------//
  void RtAccelerationStructureDX12::BuildInternal(eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC>* someGeometryDescs, eastl::vector<D3D12_RAYTRACING_INSTANCE_DESC>* someInstanceDescs, uint someFlags, const char* aName, CommandList* cmdList)
  {
    ASSERT((someGeometryDescs && !someInstanceDescs) || (!someGeometryDescs && someInstanceDescs));
    const bool isBLAS = someGeometryDescs != nullptr;
    ASSERT(isBLAS == (myType == RtAccelerationStructureType::BOTTOM_LEVEL));

    const GpuBuffer* instanceDescBuffer = nullptr;
    uint64 instanceDescBufferOffset = 0;
    if (!isBLAS)
    {
      uint64 instanceDescBufferSize = MathUtil::Align(someInstanceDescs->size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT);
      uint sizeNeeded = (uint)glm::ceil(float(instanceDescBufferSize) / sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
      someInstanceDescs->resize(sizeNeeded);
      instanceDescBuffer = cmdList->GetBuffer(instanceDescBufferOffset, GpuBufferUsage::STAGING_UPLOAD, someInstanceDescs->data(), instanceDescBufferSize, D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS asInputs = {};
    asInputs.Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(Private::GetAccelerationStructureFlags(someFlags));
    asInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    asInputs.Type = RenderCore_PlatformDX12::GetRtAccelerationStructureType(myType);
    if (isBLAS)
    {
      asInputs.NumDescs = (uint)someGeometryDescs->size();
      asInputs.pGeometryDescs = someGeometryDescs->data();
    }
    else
    {
      asInputs.InstanceDescs = instanceDescBuffer->GetDeviceAddress() + instanceDescBufferOffset;
      asInputs.NumDescs = (uint) someInstanceDescs->size();
    }
    
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO asPrebuildInfo = {};
    RenderCore::GetPlatformDX12()->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&asInputs, &asPrebuildInfo);
    ASSERT(asPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    GpuBufferProperties bufferProps;
    bufferProps.myBindFlags = (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_STORAGE;
    bufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    bufferProps.myElementSizeBytes = asPrebuildInfo.ResultDataMaxSizeInBytes;
    bufferProps.myNumElements = 1;
    bufferProps.myIsShaderWritable = true;
    myBuffer = RenderCore::CreateBuffer(bufferProps, StaticString<128>("%s_%s_buffer", aName ? aName : "Unnamed", isBLAS ? "BLAS" : "TLAS"));
    ASSERT(myBuffer != nullptr);

    GpuBufferViewProperties viewProps;
    viewProps.myFormat = DataFormat::UNKNOWN;
    viewProps.myIsRtAccelerationStructure = true;
    viewProps.myIsShaderWritable = false;
    myBufferRead = RenderCore::CreateBufferView(myBuffer, viewProps, StaticString<128>("%s_%s_bufferView", aName ? aName : "Unnamed", isBLAS ? "BLAS" : "TLAS"));
    ASSERT(myBufferRead != nullptr);

    // Actually build the BVH
    GpuBufferProperties tempBufferProps;
    tempBufferProps.myBindFlags = (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_BUILD_INPUT;
    tempBufferProps.myIsShaderWritable = true;
    tempBufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    tempBufferProps.myNumElements = 1;
    tempBufferProps.myElementSizeBytes = asPrebuildInfo.ScratchDataSizeInBytes;
    SharedPtr<GpuBuffer> buildTempBuffer = RenderCore::CreateBuffer(tempBufferProps, StaticString<128>("%s_%s_tempBuffer", aName ? aName : "Unnamed", isBLAS ? "BLAS" : "TLAS"));
    ASSERT(buildTempBuffer != nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asBuildDesc = {};
    asBuildDesc.Inputs = asInputs;
    asBuildDesc.DestAccelerationStructureData = myBuffer->GetDeviceAddress();
    asBuildDesc.ScratchAccelerationStructureData = buildTempBuffer->GetDeviceAddress();

    ID3D12GraphicsCommandList6* dx12CmdList = static_cast<CommandListDX12*>(cmdList)->GetDX12CommandList();
    dx12CmdList->BuildRaytracingAccelerationStructure(&asBuildDesc, 0, nullptr);
    const GpuResource* res = myBuffer.get();
    cmdList->ResourceUAVbarrier(&res, 1);
    RenderCore::ExecuteAndFreeCommandList(cmdList, SyncMode::BLOCKING);
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureDX12::~RtAccelerationStructureDX12()
  {

  }
//---------------------------------------------------------------------------//
}


