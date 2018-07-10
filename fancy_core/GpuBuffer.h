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
  struct GpuBufferViewProperties
  {
    GpuBufferViewProperties()
      : myFormat(DataFormat::UNKNOWN)
      , myStructureSize(0u)
      , myIsConstantBuffer(false)
      , myIsShaderWritable(false)
      , myIsStructured(false)
      , myIsRaw(false)
      , myOffset(0u)
      , mySize(~0u)
    {}

    DataFormat myFormat;
    uint myStructureSize;
    bool myIsConstantBuffer;
    bool myIsShaderWritable;
    bool myIsStructured;
    bool myIsRaw;
    uint64 myOffset;
    uint64 mySize;
  };
//---------------------------------------------------------------------------//
  struct GpuBufferView : public GpuResourceView
  {
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
