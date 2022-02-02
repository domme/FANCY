#pragma once

#include "Rendering/GpuBuffer.h"
#include "Rendering/RenderEnums.h"

#if FANCY_ENABLE_DX12

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
    uint64 GetDeviceAddress() const override;
    
    GpuResourceDataDX12* GetData() const;

  protected:
    void* Map_Internal(uint64 anOffset, uint64 aSize) const override;
    void Unmap_Internal(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const override;

    void Destroy();
	};
//---------------------------------------------------------------------------//
  class GpuBufferViewDX12 final : public GpuBufferView
  {
  public:
    GpuBufferViewDX12(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties);
    ~GpuBufferViewDX12() override;

    static bool CreateSRVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateUAVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
    static bool CreateCBVdescriptor(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties, const DescriptorDX12& aDescriptor);
  };
//---------------------------------------------------------------------------//
}

#endif