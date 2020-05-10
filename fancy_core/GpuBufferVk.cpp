#include "fancy_core_precompile.h"
#include "GpuBufferVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"
#include "CommandList.h"
#include "GpuResourceViewDataVk.h"

#if FANCY_ENABLE_VK

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
      && dataVk->myType == GpuResourceType::BUFFER 
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
    dataVk->myType = GpuResourceType::BUFFER;
    myNativeData = dataVk;

    myProperties = someProperties;
    myName = aName != nullptr ? aName : "GpuBuffer_Unnamed";

    const uint64 sizeBytes = someProperties.myElementSizeBytes * someProperties.myNumElements;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = sizeBytes;
    bufferInfo.flags = 0u;

    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    const bool hasAsyncQueues = caps.myHasAsyncCompute || caps.myHasAsyncCopy;

    bufferInfo.sharingMode = hasAsyncQueues ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    
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
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  // (RW)StructuredBuffer, (RW)ByteAddressBuffer
          | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT  // RWBuffer<Format>
          | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;  // Buffer<Format>
        readMask |= VK_ACCESS_SHADER_READ_BIT;
    }

    VkMemoryPropertyFlags memPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_WRITE)  // Upload heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      writeMask |= VK_ACCESS_HOST_WRITE_BIT;
    }
    else if (someProperties.myCpuAccess == CpuMemoryAccessType::CPU_READ)  // Readback heap
    {
      memPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      readMask |= VK_ACCESS_HOST_READ_BIT;
    }

    mySubresources = SubresourceRange(0u, 1u, 0u, 1u, 0u, 1u);

    GpuResourceHazardDataVk* hazardData = &dataVk->myHazardData;
    *hazardData = GpuResourceHazardDataVk();
    hazardData->myReadAccessMask = readMask;
    hazardData->myWriteAccessMask = writeMask;
    hazardData->myHasExclusiveQueueAccess = bufferInfo.sharingMode == VK_SHARING_MODE_EXCLUSIVE;
    
    GpuSubresourceHazardDataVk subHazardData;
    subHazardData.myContext = CommandListType::Graphics;
    subHazardData.myImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    subHazardData.myAccessMask = 0u;
    hazardData->mySubresources.resize(1u, subHazardData);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const uint queueFamilyIndices[] = 
    {
      (uint) platformVk->myQueueInfos[(uint) CommandListType::Graphics].myQueueFamilyIndex,
      (uint) platformVk->myQueueInfos[(uint) CommandListType::Compute].myQueueFamilyIndex
    };

    bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    bufferInfo.queueFamilyIndexCount = caps.myHasAsyncCompute ? ARRAY_LENGTH(queueFamilyIndices) : 1;

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
        ctx->UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
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
  VkBufferView GpuBufferViewVk::CreateVkBufferView(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties)
  {
    ASSERT(someProperties.myFormat != DataFormat::UNKNOWN && !someProperties.myIsStructured && !someProperties.myIsRaw);

    const GpuBufferProperties& bufferProps = aBuffer->GetProperties();
    ASSERT((bufferProps.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER) != 0u, "A vkBufferView can only be created for buffers created with the SHADER_BUFFER bind flag");

    VkBufferViewCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0u;
    info.buffer = static_cast<const GpuBufferVk*>(aBuffer)->GetData()->myBuffer;
    info.format = RenderCore_PlatformVk::ResolveFormat(someProperties.myFormat);
    info.offset = someProperties.myOffset;
    info.range = someProperties.mySize;

    VkBufferView bufferView;
    ASSERT_VK_RESULT(vkCreateBufferView(RenderCore::GetPlatformVk()->myDevice, &info, nullptr, &bufferView));

    return bufferView;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  GpuBufferViewVk::GpuBufferViewVk(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
    : GpuBufferView(aBuffer, someProperties)
  {
    // Creating a vkBufferView is only needed for uniform texel buffers or storage texel buffers (Buffer<T> or RWBuffer<T> in HLSL)
    // For anything else, GpuBufferViewVk just acts as a reference-holder to the GpuBuffer and stores the view-properties needed when binding
    if (myProperties.myFormat != DataFormat::UNKNOWN && !myProperties.myIsStructured && !myProperties.myIsRaw)
    {
      VkBufferView bufferView = CreateVkBufferView(aBuffer.get(), someProperties);

      GpuResourceViewDataVk nativeData;
      nativeData.myType = GpuResourceViewDataVk::Buffer;
      nativeData.myView.myBuffer = bufferView;
      myNativeData = nativeData;
    }

    
  }
//---------------------------------------------------------------------------//
  GpuBufferViewVk::~GpuBufferViewVk()
  {
    if (myNativeData.HasType<GpuResourceViewDataVk>())
    {
      const GpuResourceViewDataVk& nativeData = myNativeData.To<GpuResourceViewDataVk>();
      if (nativeData.myView.myBuffer != nullptr)
        vkDestroyBufferView(RenderCore::GetPlatformVk()->myDevice, nativeData.myView.myBuffer, nullptr);
    }
  }
//---------------------------------------------------------------------------//
  VkBufferView GpuBufferViewVk::GetBufferView() const
  {
    if (myNativeData.HasType<GpuResourceViewDataVk>())
      return myNativeData.To<GpuResourceViewDataVk>().myView.myBuffer;

    return nullptr;
  }
//---------------------------------------------------------------------------//
}

#endif // VK_SUPPORT