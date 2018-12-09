#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"
#include "GpuBufferDesc.h"
#include "GpuResourceView.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuBuffer;

  struct MappedBufferData
  {
    MappedBufferData() = default;
    MappedBufferData(const MappedBufferData& anOther) = delete;
    MappedBufferData(MappedBufferData&& anOther) = delete;
    MappedBufferData& operator= (const MappedBufferData& anOther) = delete;
    MappedBufferData& operator= (MappedBufferData&& anOther) = delete;
    ~MappedBufferData();

    const GpuBuffer* myBuffer = nullptr;
    void* myData = nullptr;
    uint64 myOffset = 0u;
    uint64 mySize = UINT64_MAX;
    GpuResourceMapMode myMapMode = GpuResourceMapMode::WRITE;
    bool myIsPersistantMap = false;
  };
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

    bool Map(MappedBufferData& aMappedDataOut, bool aIsPersistent, GpuResourceMapMode aMapMode, uint64 anOffset = 0u, uint64 aSize = UINT64_MAX) const;
    
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
