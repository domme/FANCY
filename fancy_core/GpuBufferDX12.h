#pragma once

#include "GpuBuffer.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class DescriptorDX12;
  struct GpuResourceDataDX12;
//---------------------------------------------------------------------------//
	class GpuBufferDX12 final : public GpuBuffer
	{
	public:
    GpuBufferDX12() = default;
    ~GpuBufferDX12() override;
    
    bool IsValid() const override;
    void SetName(const char* aName) override;

    void Create(const GpuBufferProperties& clParameters, const char* aName = nullptr, const void* pInitialData = nullptr) override;
    void* Map(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const override;
    void Unmap(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const override;

    GpuResourceDataDX12* GetData() const;

  protected:
    void Destroy();
	};
//---------------------------------------------------------------------------//
  class GpuBufferViewDX12 final : public GpuBufferView
  {
  public:
    GpuBufferViewDX12(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties);
    ~GpuBufferViewDX12() override;

  private:
    static bool CreateSRV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateUAV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateCBV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
  };
//---------------------------------------------------------------------------//
}
