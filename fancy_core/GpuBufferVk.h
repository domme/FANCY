#pragma once
#include "GpuBuffer.h"

#include "VkPrerequisites.h"
#include "GpuResourceDataVk.h"

namespace Fancy
{
  class GpuBufferVk final : public GpuBuffer
  {
  public:
    GpuBufferVk() = default;
    ~GpuBufferVk() override;

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const GpuBufferProperties& someProperties, const char* aName = nullptr, const void* pInitialData = nullptr) override;
    void* Map(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const override;
    void Unmap(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const override;

    GpuResourceDataVk* GetData() const;

  protected:
    void Destroy();

    VkDeviceMemory myMemory = nullptr;  // Temporary until the memory allocator is implemented for Vulkan

  };
}


