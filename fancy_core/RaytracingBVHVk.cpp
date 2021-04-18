#include "fancy_core_precompile.h"
#include "RaytracingBVHVk.h"
#include "VkPrerequisites.h"
#include "RenderCore_PlatformVk.h"
#include "GpuBufferVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Private
  {
    VkIndexType GetIndexType(DataFormat aFormat)
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

    uint GetBuildGeometryFlags(uint aSomeRaytracingBVHFlags)
    {
      uint flags = 0u;

      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::ALLOW_UPDATE)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::ALLOW_COMPACTION)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
      if (aSomeRaytracingBVHFlags & (uint)RaytracingBVHFlags::MINIMIZE_MEMORY)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
      if (flags & (uint)RaytracingBVHFlags::PREFER_FAST_BUILD)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
      if (flags & (uint)RaytracingBVHFlags::PREFER_FAST_TRACE)
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

      return flags;
    }
  }
//---------------------------------------------------------------------------//
  RaytracingBVHVk::RaytracingBVHVk(const RaytracingBVHProps& someProps, const eastl::span<RaytracingBVHGeometry>& someGeometries, const char* aName /*= nullptr*/)
    : RaytracingBVH(someProps)
  {
    ASSERT(!someGeometries.empty());
    myProps = someProps;

    const VkAccelerationStructureGeometryKHR emptyAsGeo{};
    eastl::fixed_vector<VkAccelerationStructureGeometryKHR, 64> geometriesVk(someGeometries.size(), emptyAsGeo);
    eastl::fixed_vector<uint, 64> numTrianglesPerGeo(someGeometries.size());

    for (uint i = 0u; i < (uint)someGeometries.size(); ++i)
    {
      const RaytracingBVHGeometry& geo = someGeometries[i];
      const uint vertexStride = DataFormatInfo::GetFormatInfo(geo.myVertexFormat).mySizeBytes;
      const uint indexStride = DataFormatInfo::GetFormatInfo(geo.myIndexFormat).mySizeBytes;

      VkAccelerationStructureGeometryKHR& geoVk = geometriesVk[i];
      geoVk.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;

      if (geo.myFlags & (uint)RaytracingBVHGeometryFlags::OPAQUE_GEOMETRY)
        geoVk.flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
      if (geo.myFlags & (uint)RaytracingBVHGeometryFlags::NO_DUPLICATE_ANYHIT_INVOCATION)
        geoVk.flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

      geoVk.geometryType = RenderCore_PlatformVk::GetRaytracingBVHGeometryType(geo.myType);
      if (geoVk.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR)
      {
        numTrianglesPerGeo[i] = static_cast<uint>(geo.myIndexBuffer->GetByteSize() / static_cast<uint64>(indexStride) / static_cast<uint64>(3));

        geoVk.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        geoVk.geometry.triangles.vertexFormat = RenderCore_PlatformVk::ResolveFormat(geo.myVertexFormat);
        geoVk.geometry.triangles.vertexData.deviceAddress = geo.myVertexBuffer->GetDeviceAddress() + geo.myVertexBufferOffset;
        geoVk.geometry.triangles.maxVertex = geo.myNumVertices;
        geoVk.geometry.triangles.vertexStride = vertexStride;
        geoVk.geometry.triangles.indexType = Private::GetIndexType(geo.myIndexFormat);
        geoVk.geometry.triangles.indexData.deviceAddress = geo.myIndexBuffer->GetDeviceAddress() + geo.myIndexBufferOffset;
        if (geo.myTransformBuffer)
        {
          geoVk.geometry.triangles.transformData.deviceAddress = geo.myTransformBuffer->GetDeviceAddress() + geo.myTransformBufferOffset;
        }
        else
        {
          geoVk.geometry.triangles.transformData.deviceAddress = 0;
          geoVk.geometry.triangles.transformData.hostAddress = nullptr;
        }
      }
      else
      {
        ASSERT(false, "Not implemented yet");
      }
    }

    const VkAccelerationStructureTypeKHR accelerationStructureTypeVk = RenderCore_PlatformVk::GetRaytracingBVHType(someProps.myType);

    VkAccelerationStructureBuildGeometryInfoKHR asBuildGeoInfo{};
    asBuildGeoInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    asBuildGeoInfo.type = accelerationStructureTypeVk;
    asBuildGeoInfo.flags = Private::GetBuildGeometryFlags(someProps.myFlags);
    asBuildGeoInfo.geometryCount = (uint)geometriesVk.size();
    asBuildGeoInfo.pGeometries = geometriesVk.data();

    VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo{};
    asBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    VkExt::vkGetAccelerationStructureBuildSizesKHR(
      RenderCore::GetPlatformVk()->GetDevice(),
      VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &asBuildGeoInfo,
      numTrianglesPerGeo.data(),
      &asBuildSizesInfo);

    // Create the internal buffer to hold the acceleration structure
    GpuBufferProperties bufferProps;
    bufferProps.myBindFlags = (uint)GpuBufferBindFlags::RAYTRACING_BVH_STORAGE;
    bufferProps.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
    bufferProps.myElementSizeBytes = asBuildSizesInfo.accelerationStructureSize;
    bufferProps.myNumElements = 1ull;
    bufferProps.myIsShaderWritable = true;
    myBuffer = RenderCore::CreateBuffer(bufferProps, aName ? aName : "rt acceleration structure");

    VkAccelerationStructureCreateInfoKHR asCreateInfo{};
    asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    asCreateInfo.size = asBuildSizesInfo.accelerationStructureSize;
    asCreateInfo.buffer = myBuffer->GetVkData()->myBufferData.myBuffer;
    asCreateInfo.deviceAddress = 0; // myBuffer->GetVkData()->myBufferData.myAddress;  // Only needed for replay feature
    asCreateInfo.type = accelerationStructureTypeVk;
    ASSERT_VK_RESULT(VkExt::vkCreateAccelerationStructureKHR(RenderCore::GetPlatformVk()->GetDevice(), &asCreateInfo, nullptr, &myAccelerationStructure));
  }
//---------------------------------------------------------------------------//
  RaytracingBVHVk::~RaytracingBVHVk()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void RaytracingBVHVk::Destroy()
  {
    if (myAccelerationStructure)
      VkExt::vkDestroyAccelerationStructureKHR(RenderCore::GetPlatformVk()->GetDevice(), myAccelerationStructure, nullptr);

    myBuffer.reset();

    myAccelerationStructure = nullptr;
  }
//---------------------------------------------------------------------------//
}


