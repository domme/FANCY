#include "fancy_core_precompile.h"
#include "GpuBufferVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"
#include "CommandList.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuBufferVk::~GpuBufferVk()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  bool GpuBufferVk::IsValid() const
  {
    GpuResourceDataVk* dataVk = GetData();
    return dataVk != nullptr 
      && dataVk->myType == GpuResourceCategory::BUFFER 
      && dataVk->myBuffer != nullptr;
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::SetName(const char* aName)
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::Create(const GpuBufferProperties& someProperties, const char* aName, const void* pInitialData)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();
    GpuResourceDataVk* dataVk = new GpuResourceDataVk();
    dataVk->myType = GpuResourceCategory::BUFFER;
    myNativeData = dataVk;

    myProperties = someProperties;
    myName = aName != nullptr ? aName : "GpuBuffer_Unnamed";

    const uint64 sizeBytes = someProperties.myElementSizeBytes * someProperties.myNumElements;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = sizeBytes;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    bufferInfo.flags = 0u;
    
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkAccessFlags readMask = VK_ACCESS_TRANSFER_READ_BIT;
    VkAccessFlags writeMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    if (someProperties.myIsShaderWritable)
      writeMask |= VK_ACCESS_SHADER_WRITE_BIT;

    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::CONSTANT_BUFFER)
    {
      bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      readMask |= VK_ACCESS_UNIFORM_READ_BIT;
    }
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::VERTEX_BUFFER)
    {
      bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      readMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::INDEX_BUFFER)
    {
      bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      readMask |= VK_ACCESS_INDEX_READ_BIT;
    }
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER)
    { 
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        readMask |= VK_ACCESS_SHADER_READ_BIT;
    }

    VkMemoryPropertyFlags memPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    GpuResourceState defaultState = GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;
    bool canChangeStates = true;
    if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      writeMask |= VK_ACCESS_HOST_WRITE_BIT;
    }
    else if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      defaultState = GpuResourceState::WRITE_COPY_DEST;
      readMask |= VK_ACCESS_HOST_READ_BIT;
    }

    myStateTracking = GpuResourceStateTracking();
    myStateTracking.myCanChangeStates = canChangeStates;
    myStateTracking.myDefaultState = defaultState;
    myStateTracking.myVkData.myReadAccessMask = readMask;
    myStateTracking.myVkData.myWriteAccessMask = writeMask;
    myStateTracking.myVkData.myHasExclusiveQueueAccess = bufferInfo.sharingMode == VK_SHARING_MODE_EXCLUSIVE;

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const uint queueFamilyIndices[] = 
    {
      (uint) platformVk->myQueueInfos[(uint) CommandListType::Graphics].myQueueFamilyIndex,
      (uint) platformVk->myQueueInfos[(uint) CommandListType::Compute].myQueueFamilyIndex
    };

    bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    bufferInfo.queueFamilyIndexCount = ARRAY_LENGTH(queueFamilyIndices);

    VkDevice device = platformVk->myDevice;
    ASSERT_VK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &dataVk->myBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, dataVk->myBuffer, &memRequirements);
    ASSERT(memRequirements.alignment <= UINT_MAX);
    myAlignment = (uint) memRequirements.alignment;

    const uint memoryTypeIndex = platformVk->FindMemoryTypeIndex(memRequirements, memPropertyFlags);

    // TODO: Replace memory allocation with a dedicated allocator like in DX12
    VkMemoryAllocateInfo memAllocInfo;
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    ASSERT_VK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dataVk->myMemory));
    ASSERT_VK_RESULT(vkBindBufferMemory(device, dataVk->myBuffer, dataVk->myMemory, 0));

    if (pInitialData != nullptr)
    {
      if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED);
      }
      else
      {
        CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
        ctx->ResourceBarrier(this, defaultState, GpuResourceState::WRITE_COPY_DEST);
        ctx->UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        ctx->ResourceBarrier(this, GpuResourceState::WRITE_COPY_DEST, defaultState);
        RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
      }
    }
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* GpuBufferVk::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataVk*>();
  }
//---------------------------------------------------------------------------//
  void* GpuBufferVk::Map_Internal(uint64 anOffset, uint64 aSize) const
  {
    GpuResourceDataVk* dataVk = GetData();

    const VkMemoryMapFlags mapFlags = static_cast<VkMemoryMapFlags>(0);

    void* mappedData = nullptr;
    ASSERT_VK_RESULT(vkMapMemory(RenderCore::GetPlatformVk()->myDevice, dataVk->myMemory, anOffset, aSize, mapFlags, &mappedData));
    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::Unmap_Internal(GpuResourceMapMode /*aMapMode*/, uint64 /*anOffset*/, uint64 /*aSize*/) const
  {
    GpuResourceDataVk* dataVk = GetData();
    vkUnmapMemory(RenderCore::GetPlatformVk()->myDevice, dataVk->myMemory);
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::Destroy()
  {
    if (IsValid())
    {
      VkDevice device = RenderCore::GetPlatformVk()->myDevice;
      GpuResourceDataVk* dataVk = GetData();
      vkDestroyBuffer(device, dataVk->myBuffer, nullptr);
      vkFreeMemory(device, dataVk->myMemory, nullptr);
      delete dataVk;
      dataVk = nullptr;
    }

    myNativeData.Clear();
    myProperties = GpuBufferProperties();
  }
//---------------------------------------------------------------------------//
}
