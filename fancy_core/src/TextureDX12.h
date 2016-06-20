#pragma once
#include "GpuResourceDX12.h"
#include "GpuResource.h"

#if defined (RENDERER_DX12)

#include "TextureDesc.h"
#include "DescriptorDX12.h"
#include "Descriptor.h"

namespace Fancy{ namespace Rendering{
class Descriptor;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class TextureDX12 : public GpuResource
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:

    TextureDX12();
    ~TextureDX12();
    bool operator==(const TextureDesc& aDesc) const;
    TextureDesc GetDescription() const;
    void SetFromDescription(const TextureDesc& aDesc);

    void create(const TextureParams& clDeclaration, const TextureUploadData* someInitialDatas = nullptr, uint32 aNumInitialDatas = 0u);
    void setPixelData(void* pData, uint uDataSizeBytes,
      glm::u32vec3 rectPosOffset = glm::u32vec3(0, 0, 0), glm::u32vec3 rectDimensions = glm::u32vec3(0, 0, 0));
    void* lock(GpuResoruceLockOption option = GpuResoruceLockOption::WRITE_DISCARD);
    void unlock();

    bool isDepthStencilTexture() const { return myParameters.bIsDepthStencil; }
    bool isSRGBtexture() const { return myState.isSRGB; }
    bool isLocked() const { return myState.isLocked; }
    bool isCubemap() const { return myState.isCubemap; }
    bool isArrayTexture() const { return myState.isArrayTexture; }
    bool isValid() const { return myResource != nullptr; }
        
    uint getNumDimensions() const { return myState.numDimensions; }
    const TextureParams& getParameters() const { return myParameters; }
    const String& getPath() const { return myParameters.path; }
    void setPath(const String& _aPath) { myParameters.path = _aPath; }

    const Descriptor& GetRtv() const { return myRtvDescriptor; }
    const Descriptor& GetSrv() const { return mySrvDescriptor; }
    const Descriptor& GetUav() const { return myUavDescriptor; }
    const Descriptor& GetDsv() const { return myDsvDescriptor; }
    
  protected:
    void Destroy();

    struct TextureInfos {
      TextureInfos() : isSRGB(0), isLocked(0), isArrayTexture(0), isCubemap(0),
        cachesTextureData(0), numDimensions(0) {}

      uint isSRGB : 1;
      uint isLocked : 1;
      uint isArrayTexture : 1;
      uint isCubemap : 1;
      uint cachesTextureData : 1;
      uint numDimensions : 4;
    };

    TextureParams myParameters;
    TextureInfos myState;

    Descriptor myRtvDescriptor;
    Descriptor mySrvDescriptor;
    Descriptor myUavDescriptor;
    Descriptor myDsvDescriptor;
  };

//---------------------------------------------------------------------------//
} } }

#endif

