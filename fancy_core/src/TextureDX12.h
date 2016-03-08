#pragma once
#include "GpuResourceDX12.h"

#if defined (RENDERER_DX12)

#include "TextureDesc.h"
#include "DescriptorDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class TextureDX12 : public GpuResourceDX12
  {
    friend class RendererDX12;  // Remove after backbuffers are handled through the texture class

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
    bool isValid() const { return false; }  // TODO: Implement
        
    uint getNumDimensions() const { return myState.numDimensions; }
    const TextureParams& getParameters() const { return myParameters; }
    const ObjectName& getPath() const { return myParameters.path; }
    void setPath(const String& _aPath) { myParameters.path = _aPath; }

    // DX12-Specific stuff:
    const DescriptorDX12& GetRtv() const { return myRtvDescriptor; }
    const DescriptorDX12& GetSrv() const { return mySrvDescriptor; }
    const DescriptorDX12& GetUav() const { return myUavDescriptor; }
    
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

    DescriptorDX12 myRtvDescriptor;
    DescriptorDX12 mySrvDescriptor;
    DescriptorDX12 myUavDescriptor;
  };

//---------------------------------------------------------------------------//
} } }

#endif

