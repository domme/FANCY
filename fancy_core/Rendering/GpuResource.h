#pragma once

#include "RenderEnums.h"
#include "Common/FancyCoreDefines.h"
#include "TextureData.h"

#if FANCY_ENABLE_DX12
#include "DX12/GpuResourceDataDX12.h"
#endif
#if FANCY_ENABLE_VK
#include "Vulkan/GpuResourceDataVk.h"
#endif

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct GpuResourceDestructor {
    template < typename T > void operator()( T * ptr ) const {
      delete static_cast< GpuResource * >( ptr );
    }
  };
  //---------------------------------------------------------------------------//
  class GpuResource {
  public:
    friend struct GpuResourceDestructor;
    friend class RenderOutputDX12;
    friend class RenderOutputVk;

    GpuResource( GpuResource && anOther ) = default;

    explicit GpuResource( GpuResourceType aType ) : mySubresources( 0, 1u, 0u, 1u, 0u, 1u ), myType( aType ) {}

    void operator=( GpuResource && anOtherResource ) noexcept {
      myType = anOtherResource.myType;
      myName = anOtherResource.myName;
      mySubresources = anOtherResource.mySubresources;
#if FANCY_ENABLE_DX12
      myDx12Data = anOtherResource.myDx12Data;
#endif
#if FANCY_ENABLE_VK
      myVkData = anOtherResource.myVkData;
#endif
    }

    static uint CalcSubresourceIndex( uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices,
                                      uint aPlaneIndex );
    static uint CalcNumSubresources( uint aNumMips, uint aNumArraySlices, uint aNumPlanes );

    uint                     GetSubresourceIndex( const SubresourceLocation & aSubresourceLocation ) const;
    SubresourceLocation      GetSubresourceLocation( uint aSubresourceIndex ) const;
    const SubresourceRange & GetSubresources() const {
      return mySubresources;
    }
    const char * GetName() const {
      return myName.c_str();
    }

#if FANCY_ENABLE_DX12
    const GpuResourceDataDX12 * GetDX12Data() const {
      return &myDx12Data;
    }
    GpuResourceDataDX12 * GetDX12Data() {
      return &myDx12Data;
    }
#endif
#if FANCY_ENABLE_VK
    const GpuResourceDataVk * GetVkData() const {
      return &myVkData;
    }
    GpuResourceDataVk * GetVkData() {
      return &myVkData;
    }
#endif

    bool IsBuffer() const {
      return myType == GpuResourceType::BUFFER;
    }
    bool IsTexture() const {
      return myType == GpuResourceType::TEXTURE;
    }
    GpuResourceType GetType() const {
      return myType;
    }

    virtual bool IsValid() const {
      return false;
    }
    virtual void SetName( const char * aName ) {
      myName = aName;
    }

    SubresourceRange mySubresources;
    eastl::string    myName;
    GpuResourceType  myType;

#if FANCY_ENABLE_DX12
    GpuResourceDataDX12 myDx12Data;
#endif
#if FANCY_ENABLE_VK
    GpuResourceDataVk myVkData;
#endif
  protected:
    virtual ~GpuResource() = default;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
