#pragma once
#include "GpuResourceDX12.h"

#include "DescriptorDX12.h"
#include "Descriptor.h"
#include "Texture.h"

namespace Fancy{ namespace Rendering{
class Descriptor;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class TextureDX12 : public Texture, public GpuResourceDX12
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12();
    ~TextureDX12() override;
    
    void Create(const TextureParams& clDeclaration, const TextureUploadData* someInitialDatas = nullptr, uint32 aNumInitialDatas = 0u) override;
    
    bool isValid() const { return myResource != nullptr; }

    const Descriptor& GetRtv() const { return myRtvDescriptor; }
    const Descriptor& GetSrv() const { return mySrvDescriptor; }
    const Descriptor& GetUav() const { return myUavDescriptor; }
    const Descriptor& GetDsv() const { return myDsvDescriptor; }
    
  protected:
    void Destroy();

    Descriptor myRtvDescriptor;
    Descriptor mySrvDescriptor;
    Descriptor myUavDescriptor;
    Descriptor myDsvDescriptor;
  };

//---------------------------------------------------------------------------//
} } }
