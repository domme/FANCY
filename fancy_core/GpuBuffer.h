#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "GpuBufferDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuBuffer : public GpuResource
  {
  public:
    GpuBuffer();
    virtual ~GpuBuffer();

    bool operator==(const GpuBufferDesc& aDesc) const;
    GpuBufferDesc GetDescription() const;

    uint GetSizeBytes() const { return myParameters.uNumElements * myParameters.uElementSizeBytes; }
    uint GetNumElements() const { return myParameters.uNumElements; }
    GpuBufferCreationParams GetParameters() const { return myParameters; }
    uint GetAlignment() const { return myAlignment; }

    virtual void Create(const GpuBufferCreationParams& clParameters,const void* pInitialData = nullptr) = 0;
    virtual void* Lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u) const = 0;
    virtual void Unlock(uint anOffsetElements = 0u, uint aNumElements = 0u) const = 0;
    
  protected:
    uint myAlignment;
    GpuBufferCreationParams myParameters;

  };
//---------------------------------------------------------------------------//
}
