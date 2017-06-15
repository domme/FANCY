#pragma once
#include "GpuResourceDX12.h"

#include "DescriptorDX12.h"
#include "Texture.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class TextureDX12 : public Texture, public GpuResourceDX12
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12();
    ~TextureDX12() override;
    bool IsValid() const override { return myResource != nullptr; }

    void Create(const TextureParams& clDeclaration, const TextureUploadData* someInitialDatas = nullptr, uint32 aNumInitialDatas = 0u) override;

    const DescriptorDX12* GetSrv() const override { return &mySrvDescriptor; }
    const DescriptorDX12* GetUav() const override { return &myUavDescriptor; }
    const DescriptorDX12* GetCbv() const override { return nullptr; }
    const DescriptorDX12* GetRtv() const override { return &myRtvDescriptor; }
    const DescriptorDX12* GetDsv() const override { return &myDsvDescriptor; }
    
  protected:
    void Destroy();

    DescriptorDX12 myRtvDescriptor;
    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
    DescriptorDX12 myDsvDescriptor;
  };

//---------------------------------------------------------------------------//
} } }
