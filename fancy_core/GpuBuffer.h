#pragma once

#include "GpuResource.h"
#include "GpuBufferProperties.h"
#include "GpuResourceView.h"
#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuBuffer : public GpuResource
  {
  public:
    GpuBuffer();
    virtual ~GpuBuffer() = default;

    virtual void Create(const GpuBufferProperties& clParameters, const char* aName = nullptr, const void* pInitialData = nullptr) = 0;
    virtual uint64 GetDeviceAddress() const = 0;
    
    void* Map(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const;
    void Unmap(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const;

    const GpuBufferProperties& GetProperties() const { return myProperties; }
    uint GetAlignment() const { return myAlignment; }
    uint64 GetByteSize() const { return myProperties.myNumElements * myProperties.myElementSizeBytes; }

  protected:
    virtual void* Map_Internal(uint64 anOffset, uint64 aSize) const = 0;
    virtual void Unmap_Internal(GpuResourceMapMode aMapMode, uint64 anOffset, uint64 aSize) const = 0;

    uint myAlignment;
    GpuBufferProperties myProperties;
  };
//---------------------------------------------------------------------------//
  class GpuBufferView : public GpuResourceView
  {
  public:
    GpuBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties);

    const GpuBufferViewProperties& GetProperties() const { return myProperties; }
    GpuBuffer* GetBuffer() const { return static_cast<GpuBuffer*>(myResource.get()); }

  protected:
    GpuBufferViewProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
