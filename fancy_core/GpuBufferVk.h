#pragma once
#include "GpuBuffer.h"

#include "VkPrerequisites.h"
#include "GpuResourceDataVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuBufferVk final : public GpuBuffer
  {
  public:
    GpuBufferVk() = default;
    ~GpuBufferVk() override;

    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const GpuBufferProperties& someProperties, const char* aName = nullptr, const void* pInitialData = nullptr) override;

    GpuResourceDataVk* GetData() const;

  protected:
    void* Map_Internal(uint64 anOffset, uint64 aSize) const override;
    void Unmap_Internal(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const override;

    void Destroy();
  };
//---------------------------------------------------------------------------//
  class GpuBufferViewVk final : public GpuBufferView
  {
  public:
    static VkBufferView CreateVkBufferView(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties);

    GpuBufferViewVk(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties);
    ~GpuBufferViewVk() override;

    VkBufferView GetBufferView() const;
  };
//---------------------------------------------------------------------------//
}


