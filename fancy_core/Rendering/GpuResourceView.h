#pragma once

#include "GpuResource.h"

#if FANCY_ENABLE_DX12
#include "DX12/GpuResourceViewDataDX12.h"
#endif

namespace Fancy {
  //---------------------------------------------------------------------------//
  enum class GpuResourceViewType { NONE = 0, CBV, SRV, SRV_RT_AS, UAV, DSV, RTV };
  //---------------------------------------------------------------------------//
  struct GpuResourceViewDestructor {
    template < typename T > void operator()( T * ptr ) const {
      delete static_cast< GpuResourceView * >( ptr );
    }
  };
  //---------------------------------------------------------------------------//
  class GpuResourceView {
  public:
    friend struct GpuResourceViewDestructor;

    explicit GpuResourceView( GpuResource * aResource, const char * aName )
        : myResource( aResource ), myCoversAllSubresources( true ), myType( GpuResourceViewType::NONE ),
          myGlobalDescriptorIndex( UINT_MAX ), myName( aName != nullptr ? aName : "" ) {}

    GpuResource * GetResource() const {
      return myResource;
    }
    const SubresourceRange & GetSubresourceRange() const {
      return mySubresourceRange;
    }
    GpuResourceViewType GetType() const {
      return myType;
    }
    uint GetGlobalDescriptorIndex() const {
      return myGlobalDescriptorIndex;
    }

#if FANCY_ENABLE_DX12
    GpuResourceViewDataDX12 myDX12Data;
#endif

    SubresourceRange    mySubresourceRange;
    GpuResource *       myResource;
    bool                myCoversAllSubresources;
    GpuResourceViewType myType;
    uint                myGlobalDescriptorIndex;
    eastl::string       myName;

  protected:
    virtual ~GpuResourceView() = default;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
