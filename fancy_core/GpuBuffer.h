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
    virtual void* Map(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const = 0;
    virtual void Unmap(GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const = 0;

    const GpuBufferProperties& GetProperties() const { return myProperties; }
    uint GetAlignment() const { return myAlignment; }
    uint64 GetByteSize() const { return myProperties.myNumElements * myProperties.myElementSizeBytes; }
    uint GetNumSubresources() const override{ return 1u; }

  protected:
    uint myAlignment;
    GpuBufferProperties myProperties;
  };
//---------------------------------------------------------------------------//
  class GpuBufferView : public GpuResourceView
  {
  public:
    GpuBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
      : GpuResourceView(std::static_pointer_cast<GpuResource>(aBuffer))
      , myProperties(someProperties)
    { }

    const GpuBufferViewProperties& GetProperties() const { return myProperties; }
    GpuBuffer* GetBuffer() const { return static_cast<GpuBuffer*>(myResource.get()); }

  protected:
    GpuBufferViewProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
