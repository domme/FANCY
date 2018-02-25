#pragma once
#include "DescriptorDX12.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureDX12 : public Texture
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12();
    ~TextureDX12() override;

    void Create(const TextureParams& clDeclaration, const TextureUploadData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;

    const DescriptorDX12* GetSrv() const { return &mySrvDescriptor; }
    const DescriptorDX12* GetUav() const { return &myUavDescriptor; }
    const DescriptorDX12* GetRtv() const { return &myRtvDescriptor; }
    const DescriptorDX12* GetDsv() const { return &myDsvDescriptor; }

    const DescriptorDX12* GetDescriptor(DescriptorType aType, uint anIndex = 0u) const override;
    
  protected:
    void Destroy();

    DescriptorDX12 myRtvDescriptor;
    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
    DescriptorDX12 myDsvDescriptor;
    DescriptorDX12 myDsvDescriptorReadOnly;
    DescriptorDX12 mySrvDescriptorDepth;
    DescriptorDX12 mySrvDescriptorStencil;
  };

//---------------------------------------------------------------------------//
}
