#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "GpuBufferDesc.h"
#include "GpuResourceView.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuBuffer : public GpuResource
  {
  public:
    GpuBuffer();
    virtual ~GpuBuffer();

    const GpuBufferProperties& GetProperties() const { return myProperties; }
    uint GetAlignment() const { return myAlignment; }

    uint64 GetByteSize() const { return myProperties.myNumElements * myProperties.myElementSizeBytes; }
    uint64 GetAllocatedByteSize() const { return MathUtil::Align(GetByteSize(), myAlignment); }

    virtual void Create(const GpuBufferProperties& clParameters,const void* pInitialData = nullptr) = 0;
    virtual void* Lock(GpuResoruceLockOption eLockOption, uint64 uOffsetElements = 0u, uint64 uNumElements = 0u) const = 0;
    virtual void Unlock(uint64 anOffsetElements = 0u, uint64 aNumElements = 0u) const = 0;
    
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
