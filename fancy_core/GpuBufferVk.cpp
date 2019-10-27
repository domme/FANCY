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

    myNumSubresources = 1u;
    myNumSubresourcesPerPlane = 1u;
    myNumPlanes = 1u;

    const uint64 sizeBytes = someProperties.myElementSizeBytes * someProperties.myNumElements;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = sizeBytes;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    bufferInfo.flags = 0u;
    
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::CONSTANT_BUFFER)
      bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::VERTEX_BUFFER)
      bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::INDEX_BUFFER)
      bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (someProperties.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER)
      bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const uint queueFamilyIndices[] = 
    {
      platformVk->myQueueInfos[(uint) CommandListType::Graphics].myQueueFamilyIndex,
      platformVk->myQueueInfos[(uint) CommandListType::Compute].myQueueFamilyIndex
    };

    bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    bufferInfo.queueFamilyIndexCount = ARRAY_LENGTH(queueFamilyIndices);

    VkDevice device = platformVk->myDevice;
    ASSERT_VK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &dataVk->myBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, dataVk->myBuffer, &memRequirements);
    myAlignment = memRequirements.alignment;


    VkMemoryPropertyFlags memPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    GpuResourceState defaultState = GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;
    bool canChangeStates = true;
    if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      canChangeStates = false;
    }
    else if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      defaultState = GpuResourceState::WRITE_COPY_DEST;
      canChangeStates = false;
    }

    const VkPhysicalDeviceMemoryProperties& deviceMemProps = platformVk->GetPhysicalDeviceMemoryProperties();
    
    uint memoryTypeIndex = UINT_MAX;
    for (uint i = 0u; i < deviceMemProps.memoryTypeCount; ++i)
    {
      const VkMemoryType& memType = deviceMemProps.memoryTypes[i];
      if ((memRequirements.memoryTypeBits & (1 << i)) && 
        memType.propertyFlags )
    }

    myStateTracking = GpuResourceStateTracking();
    myStateTracking.myCanChangeStates = canChangeStates;
    myStateTracking.myDefaultState = defaultState;

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
  void* GpuBufferVk::Map(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::Unmap(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const
  {
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* GpuBufferVk::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataVk*>();
  }
//---------------------------------------------------------------------------//
  void GpuBufferVk::Destroy()
  {
    if (IsValid())
    {
      VkDevice device = RenderCore::GetPlatformVk()->myDevice;
      GpuResourceDataVk* dataVk = GetData();
      vkDestroyBuffer(device, dataVk->myBuffer, nullptr);
      delete dataVk;

      vkFreeMemory(device, myMemory, nullptr);
      myMemory = nullptr;
    }

    myNativeData.Clear();
    myProperties = GpuBufferProperties();
  }
//---------------------------------------------------------------------------//
}
