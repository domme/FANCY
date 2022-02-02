#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "RtAccelerationStructureVk.h"

#include "Rendering/CommandList.h"
#include "CommandListVk.h"
#include "VkPrerequisites.h"
#include "RenderCore_PlatformVk.h"
#include "GpuBufferVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Private
  {
    static VkIndexType GetIndexType(DataFormat aFormat)
    {
      if (aFormat == DataFormat::R_32UI)
        return VK_INDEX_TYPE_UINT32;
      if (aFormat == DataFormat::R_16UI)
        return VK_INDEX_TYPE_UINT16;
      if (aFormat == DataFormat::R_8UI)
        return VK_INDEX_TYPE_UINT8_EXT;

      ASSERT(false, "Invalid index format");
      return VK_INDEX_TYPE_UINT32;
    }

    static uint GetAccelerationStructureFlags(uint someFlags)
    {
      uint flags = 0u;

      if (someFlags & (uint)RtAccelerationStructureFlags::ALLOW_UPDATE)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
      if (someFlags & (uint)RtAccelerationStructureFlags::ALLOW_COMPACTION)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
      if (someFlags & (uint)RtAccelerationStructureFlags::MINIMIZE_MEMORY)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
      if (someFlags & (uint)RtAccelerationStructureFlags::PREFER_FAST_BUILD)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
      if (someFlags & (uint)RtAccelerationStructureFlags::PREFER_FAST_TRACE)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

      return flags;
    }

    static void GetBLASGeometryDescs(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, CommandList* cmdList, eastl::vector<VkAccelerationStructureGeometryKHR>& geometryDescs, eastl::vector<uint>& numPrimitivesList)
    {
      geometryDescs.reserve(aNumGeometries);
      numPrimitivesList.reserve(aNumGeometries);

      for (uint i = 0u; i < aNumGeometries; ++i)
      {
        VkAccelerationStructureGeometryKHR& geoDescVk = geometryDescs.push_back();

        memset(&geoDescVk, 0, sizeof(geoDescVk));
        geoDescVk.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;

        const RtAccelerationStructureGeometryData& geoInfo = someGeometries[i];
        if (geoInfo.myFlags & (uint)RtAccelerationStructureGeometryFlags::OPAQUE_GEOMETRY)
          geoDescVk.flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
        if (geoInfo.myFlags & (uint)RtAccelerationStructureGeometryFlags::NO_DUPLICATE_ANYHIT_INVOCATION)
          geoDescVk.flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

        geoDescVk.geometryType = RenderCore_PlatformVk::GetRaytracingBVHGeometryType(geoInfo.myType);
        if (geoDescVk.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR)
        {
          ASSERT(geoInfo.myNumVertices > 0);

          const DataFormatInfo& vertexFormatInfo = DataFormatInfo::GetFormatInfo(geoInfo.myVertexFormat);
          const uint vertexComponentSize = vertexFormatInfo.mySizeBytes / vertexFormatInfo.myNumComponents;
          const uint vertexStride = glm::max(geoInfo.myVertexStride, vertexFormatInfo.mySizeBytes);
          ASSERT(MathUtil::IsAligned(vertexStride, vertexComponentSize)); // Stride must be a multiple of the component size

          VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
          if (geoInfo.myIndexFormat == DataFormat::R_16UI)
            indexType = VK_INDEX_TYPE_UINT16;
          else if (geoInfo.myIndexFormat == DataFormat::R_32UI)
            indexType = VK_INDEX_TYPE_UINT32;
          else
            ASSERT(false, "Unsupported index format");

          const uint indexStride = indexType == VK_INDEX_TYPE_UINT16 ? 2 : 4;
          const uint transformStride = sizeof(glm::float3x4);

          const uint64 vertexBufferAddress = geoInfo.myVertexData.GetGpuBufferAddress(cmdList, vertexComponentSize);
          const uint64 indexBufferAddress = geoInfo.myIndexData.GetGpuBufferAddress(cmdList, indexStride);
          const uint64 transformBufferAddress = geoInfo.myTransformData.GetGpuBufferAddress(cmdList, 16u);

          geoDescVk.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
          geoDescVk.geometry.triangles.pNext = nullptr;
          geoDescVk.geometry.triangles.vertexFormat = RenderCore_PlatformVk::ResolveFormat(geoInfo.myVertexFormat);
          geoDescVk.geometry.triangles.vertexData.deviceAddress = vertexBufferAddress;
          geoDescVk.geometry.triangles.vertexStride = vertexStride;
          geoDescVk.geometry.triangles.maxVertex = geoInfo.myNumVertices - 1;
          geoDescVk.geometry.triangles.indexType = indexType;
          geoDescVk.geometry.triangles.indexData.deviceAddress = indexBufferAddress;
          geoDescVk.geometry.triangles.transformData.deviceAddress = transformBufferAddress;
          
          numPrimitivesList.push_back(geoInfo.myNumIndices / 3);
        }
        else
        {
          ASSERT(geoDescVk.geometryType == VK_GEOMETRY_TYPE_AABBS_KHR);  // TYPE_INSTANCES is not supported here. Is implicitly set when building a TLAS
          const uint64 aabbBufferAddress = geoInfo.myProcedural_AABBData.GetGpuBufferAddress(cmdList, 8u);

          geoDescVk.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
          geoDescVk.geometry.aabbs.pNext = nullptr;
          geoDescVk.geometry.aabbs.data.deviceAddress = aabbBufferAddress;
          geoDescVk.geometry.aabbs.stride = 24u; // 6 Floats for min.xyz and max.xyz

          numPrimitivesList.push_back(16); // Not sure if this is correct... for AABBs do the triangles count or is the whole AABB one primitive?
        }
      }
    }

    static void GetTLASInstanceDescs(const RtAccelerationStructureInstanceData* someInstanceDatas, uint aNumInstances, eastl::vector<VkAccelerationStructureInstanceKHR>& someInstanceDescsOut)
    {
      someInstanceDescsOut.reserve(aNumInstances);

      for (uint i = 0u; i < aNumInstances; ++i)
      {
        const RtAccelerationStructureInstanceData& instanceData = someInstanceDatas[i];
        VkAccelerationStructureInstanceKHR& instanceVk = someInstanceDescsOut.push_back();
        memset(&instanceVk, 0, sizeof(instanceVk));

        if (instanceData.myFlags & RT_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
          instanceVk.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
          instanceVk.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_FORCE_OPAQUE)
          instanceVk.flags |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
        if (instanceData.myFlags & RT_INSTANCE_FLAG_FORCE_NONE_OPAQUE)
          instanceVk.flags |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;

        instanceVk.instanceShaderBindingTableRecordOffset = instanceData.mySbtHitGroupOffset;
        instanceVk.instanceCustomIndex = instanceData.myInstanceId;
        instanceVk.mask = instanceData.myInstanceMask;

        for (int row = 0; row < 3; ++row)
          for (int col = 0; col < 4; ++col)
            instanceVk.transform.matrix[row][col] = instanceData.myTransform[row][col];

        const GpuBufferVk* blasBuffer = static_cast<const GpuBufferVk*>(instanceData.myInstanceBLAS->GetBuffer());
        instanceVk.accelerationStructureReference = blasBuffer->GetAccelerationStructureAddress();
      }
    }
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureVk::RtAccelerationStructureVk(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint someFlags, const char* aName)
    : RtAccelerationStructure(RtAccelerationStructureType::BOTTOM_LEVEL, aName)
  {
    CommandList* cmdList = RenderCore::BeginCommandList(CommandListType::Graphics);
    eastl::vector<VkAccelerationStructureGeometryKHR> geometryDescs;
    eastl::vector<uint> numPrimitivesList;
    Private::GetBLASGeometryDescs(someGeometries, aNumGeometries, cmdList, geometryDescs, numPrimitivesList);
    BuildInternal(&geometryDescs, &numPrimitivesList, nullptr, someFlags, aName, cmdList);
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureVk::RtAccelerationStructureVk(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags, const char* aName)
    : RtAccelerationStructure(RtAccelerationStructureType::TOP_LEVEL, aName)
  {
    eastl::vector<VkAccelerationStructureInstanceKHR> instanceDescs;
    Private::GetTLASInstanceDescs(someInstances, aNumInstances, instanceDescs);

    CommandList* cmdList = RenderCore::BeginCommandList(CommandListType::Graphics);
    BuildInternal(nullptr, nullptr, &instanceDescs, someFlags, aName, cmdList);
  }
//---------------------------------------------------------------------------//
  void RtAccelerationStructureVk::BuildInternal(eastl::vector<VkAccelerationStructureGeometryKHR>* someGeometryDescs,
    eastl::vector<uint>* somePrimitiveCounts,
    eastl::vector<VkAccelerationStructureInstanceKHR>* someInstanceDescs,
    uint someFlags, const char* aName, CommandList* cmdList)
  {
    ASSERT(somePrimitiveCounts || !someGeometryDescs);
    ASSERT(!somePrimitiveCounts || somePrimitiveCounts->size() == someGeometryDescs->size());
    ASSERT((someGeometryDescs && !someInstanceDescs) || (!someGeometryDescs && someInstanceDescs));

    const bool isBLAS = someGeometryDescs != nullptr;
    ASSERT(isBLAS == (myType == RtAccelerationStructureType::BOTTOM_LEVEL));

    Destroy();

    const GpuBuffer* instanceDescBuffer = nullptr;
    uint64 instanceDescBufferOffset = 0;
    VkAccelerationStructureGeometryKHR instancesGeoDesc;
    VkAccelerationStructureGeometryInstancesDataKHR instancesData;
    uint numInstances = 0;
    if (!isBLAS)
    {
      uint64 instanceDescBufferSize = MathUtil::Align(someInstanceDescs->size() * sizeof(VkAccelerationStructureInstanceKHR), 16u);
      uint sizeNeeded = (uint)glm::ceil(float(instanceDescBufferSize) / sizeof(VkAccelerationStructureInstanceKHR));
      someInstanceDescs->resize(sizeNeeded);
      instanceDescBuffer = cmdList->GetBuffer(instanceDescBufferOffset, GpuBufferUsage::STAGING_UPLOAD_RT_BUILD_INPUT, someInstanceDescs->data(), instanceDescBufferSize, 16u);

      instancesData = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
      instancesData.data.deviceAddress = instanceDescBuffer->GetDeviceAddress() + instanceDescBufferOffset;

      instancesGeoDesc = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
      instancesGeoDesc.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
      instancesGeoDesc.geometry.instances = instancesData;

      numInstances = (uint) someInstanceDescs->size();
    }

    VkAccelerationStructureBuildGeometryInfoKHR asBuildInfo { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
    asBuildInfo.flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(Private::GetAccelerationStructureFlags(someFlags));
    asBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;  // TODO: Support update

    const uint* maxPrimitiveCounts;
    if (isBLAS)
    {
      asBuildInfo.geometryCount = (uint) someGeometryDescs->size();
      asBuildInfo.pGeometries = someGeometryDescs->data();
      asBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
      maxPrimitiveCounts = somePrimitiveCounts->data();
    }
    else
    {
      asBuildInfo.geometryCount = 1;
      asBuildInfo.pGeometries = &instancesGeoDesc;
      asBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
      maxPrimitiveCounts = &numInstances;
    }

    VkDevice device = RenderCore::GetPlatformVk()->GetDevice();

    VkAccelerationStructureBuildSizesInfoKHR asSizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
    VkExt::vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asBuildInfo, maxPrimitiveCounts, &asSizeInfo);

    GpuBufferProperties bufferProps;
    bufferProps.myBindFlags = (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_STORAGE;
    bufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    bufferProps.myElementSizeBytes = asSizeInfo.accelerationStructureSize;
    bufferProps.myNumElements = 1;
    bufferProps.myIsShaderWritable = true;
    myBuffer = RenderCore::CreateBuffer(bufferProps, StaticString<128>("%s_%s_buffer", aName ? aName : "Unnamed", isBLAS ? "BLAS" : "TLAS"));
    ASSERT(myBuffer != nullptr);
    
    VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
    asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    asCreateInfo.size = asSizeInfo.accelerationStructureSize;
    asCreateInfo.createFlags = 0;
    asCreateInfo.buffer = myBuffer->GetVkData()->myBufferData.myBuffer;
    asCreateInfo.type = asBuildInfo.type;
    asCreateInfo.offset = 0;
    asCreateInfo.deviceAddress = 0; // Would only be needed for replay
    ASSERT_VK_RESULT(VkExt::vkCreateAccelerationStructureKHR(device, &asCreateInfo, nullptr, &myAccelerationStructure));

    // Actually build the BVH
    GpuBufferProperties tempBufferProps;
    tempBufferProps.myBindFlags = (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_BUILD_INPUT;
    tempBufferProps.myIsShaderWritable = true;
    tempBufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    tempBufferProps.myNumElements = 1;
    tempBufferProps.myElementSizeBytes = asSizeInfo.buildScratchSize;
    SharedPtr<GpuBuffer> buildTempBuffer = RenderCore::CreateBuffer(tempBufferProps, StaticString<128>("%s_%s_tempBuffer", aName ? aName : "Unnamed", isBLAS ? "BLAS" : "TLAS"));
    ASSERT(buildTempBuffer != nullptr);

    asBuildInfo.dstAccelerationStructure = myAccelerationStructure;
    asBuildInfo.scratchData.deviceAddress = buildTempBuffer->GetDeviceAddress();

    eastl::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRangeInfos;
    if (isBLAS)
    {
      buildRangeInfos.reserve(somePrimitiveCounts->size());
      for (uint i = 0, e = (uint) somePrimitiveCounts->size(); i < e; ++i)
      {
        VkAccelerationStructureBuildRangeInfoKHR& rangeInfo = buildRangeInfos.push_back();
        rangeInfo.primitiveCount = (*somePrimitiveCounts)[i];
        rangeInfo.primitiveOffset = 0;
        rangeInfo.firstVertex = 0;
        rangeInfo.transformOffset = 0;
      }
    }
    else
    {
      VkAccelerationStructureBuildRangeInfoKHR& rangeInfo = buildRangeInfos.push_back();
      rangeInfo.primitiveCount = 1;
      rangeInfo.primitiveOffset = 0;
      rangeInfo.firstVertex = 0;
      rangeInfo.transformOffset = 0;
    }

    VkAccelerationStructureBuildRangeInfoKHR* buldRangeInfosBuffer = buildRangeInfos.data();
        
    VkCommandBuffer cmdBuffer = static_cast<CommandListVk*>(cmdList)->GetCommandBuffer();
    VkExt::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1u, &asBuildInfo, &buldRangeInfosBuffer);
    RenderCore::ExecuteAndFreeCommandList(cmdList, SyncMode::BLOCKING);
    
    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
    accelerationDeviceAddressInfo.accelerationStructure = myAccelerationStructure;
    myBuffer->GetVkData()->myBufferData.myAccelerationStructureAddress = VkExt::vkGetAccelerationStructureDeviceAddressKHR(device, &accelerationDeviceAddressInfo);
    myBuffer->GetVkData()->myBufferData.myAccelerationStructure = myAccelerationStructure;

    if (!isBLAS)
    {
      GpuBufferViewProperties viewProps;
      viewProps.myFormat = DataFormat::UNKNOWN;
      viewProps.myIsRtAccelerationStructure = true;
      viewProps.myIsShaderWritable = false;
      myTopLevelBufferRead = RenderCore::CreateBufferView(myBuffer, viewProps, StaticString<128>("%s_TLAS_bufferView", aName ? aName : "Unnamed"));
      ASSERT(myTopLevelBufferRead != nullptr);
    }
  }
//---------------------------------------------------------------------------//
  RtAccelerationStructureVk::~RtAccelerationStructureVk()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void RtAccelerationStructureVk::Destroy()
  {
    if (myAccelerationStructure)
      VkExt::vkDestroyAccelerationStructureKHR(RenderCore::GetPlatformVk()->GetDevice(), myAccelerationStructure, nullptr);

    myAccelerationStructure = nullptr;

    myBuffer.reset();
    myTopLevelBufferRead.reset();
  }
//---------------------------------------------------------------------------//
}

#endif