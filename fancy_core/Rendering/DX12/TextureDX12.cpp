#include "fancy_core_precompile.h"
#include "TextureDX12.h"

#include "Rendering/DataFormat.h"
#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"

#include "Common/StringUtil.h"

#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDataDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  TextureDX12::TextureDX12(GpuResource&& aResource, const TextureProperties& someProperties, bool aIsPresentable)
    : Texture(std::move(aResource), someProperties, aIsPresentable)
  {
  }
//---------------------------------------------------------------------------//
  TextureDX12::~TextureDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  bool TextureDX12::IsValid() const
  {
    return GetData() != nullptr && GetData()->myResource.Get() != nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::SetName(const char* aName)
  {
    Texture::SetName(aName);

    if (GpuResourceDataDX12* dataDx12 = GetData())
    {
      eastl::wstring wName = StringUtil::ToWideString(myName);
      dataDx12->myResource->SetName(wName.c_str());
    }
  }
//---------------------------------------------------------------------------//
  GpuResourceDataDX12* TextureDX12::GetData() const
  {
    return !myNativeData.has_value() ? nullptr : const_cast<GpuResourceDataDX12*>(eastl::any_cast<GpuResourceDataDX12>(&myNativeData));
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Create(const TextureProperties& someProperties, const char* aName /* = nullptr */, const TextureSubData* someInitialDatas /* = nullptr */, uint aNumInitialDatas /*= 0u*/)
  {
    Destroy();
    GpuResourceDataDX12 dataDx12;
    
    myProperties = someProperties;
    myName = aName != nullptr ? aName : "Texture_Unnamed";

    bool isArray, isCubemap;
    D3D12_RESOURCE_DIMENSION dimension = Adapter::ResolveResourceDimension(someProperties.myDimension, isCubemap, isArray);
    
    ASSERT(dimension != D3D12_RESOURCE_DIMENSION_BUFFER && dimension != D3D12_RESOURCE_DIMENSION_UNKNOWN, "Invalid texture resource dimension");
    ASSERT(!isCubemap || someProperties.myDepthOrArraySize % 6 == 0, "A cubemap needs at least 6 faces");
    ASSERT(someProperties.myWidth > 0u, "Texture needs a nonzero width");
    ASSERT(someProperties.myHeight > 0u || dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D, "Non-1D textures always need a nonzero height" )
    ASSERT(someProperties.myDepthOrArraySize > 0 || (!isArray && dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D), "Array or 3D-texture need a nonzero depthOrArraySize");
    ASSERT(!someProperties.bIsDepthStencil || !someProperties.myIsShaderWritable, "Shader writable and depthstencil are mutually exclusive");

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myProperties.myFormat);

    myProperties.myDepthOrArraySize = glm::max(1u, myProperties.myDepthOrArraySize);
    
    DXGI_FORMAT dxgiFormat = RenderCore_PlatformDX12::ResolveFormat(myProperties.myFormat);
    if (!myProperties.myPreferTypedFormat)
    {
      if (myProperties.bIsDepthStencil)
      {
        dxgiFormat = RenderCore_PlatformDX12::GetDepthStencilTextureFormat(dxgiFormat);
      }
      else
      {
        DXGI_FORMAT typelessFormat = RenderCore_PlatformDX12::GetTypelessFormat(dxgiFormat);
        if (typelessFormat != DXGI_FORMAT_UNKNOWN)
          dxgiFormat = typelessFormat;
      }
    }

    const uint minSide = (myProperties.myDimension == GpuResourceDimension::TEXTURE_3D) ? glm::min(myProperties.myWidth, myProperties.myHeight, myProperties.myDepthOrArraySize) : glm::min(myProperties.myWidth, myProperties.myHeight);
    const uint maxNumMipLevels = 1u + static_cast<uint>(glm::floor(glm::log2(minSide)));
    myProperties.myNumMipLevels = glm::max(1u, glm::min(myProperties.myNumMipLevels, maxNumMipLevels));

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = dimension;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = glm::max(1u, static_cast<uint>(myProperties.myWidth));
    resourceDesc.Height = glm::max(1u, static_cast<uint>(myProperties.myHeight));
    resourceDesc.DepthOrArraySize = (uint16) glm::max(1u, static_cast<uint>(myProperties.myDepthOrArraySize));
    resourceDesc.MipLevels = (uint16) myProperties.myNumMipLevels;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Format = dxgiFormat;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (myProperties.bIsDepthStencil)
    {
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
      if (myProperties.myIsShaderWritable)
          resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      if (myProperties.myIsRenderTarget)
          resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    D3D12_RESOURCE_STATES readStateMask = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
    D3D12_RESOURCE_STATES initialStates = readStateMask;
    D3D12_RESOURCE_STATES writeStateMask = D3D12_RESOURCE_STATE_COPY_DEST;
    if (myProperties.myIsShaderWritable)
      writeStateMask |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (myProperties.bIsDepthStencil)
    {
      writeStateMask |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
      readStateMask |= D3D12_RESOURCE_STATE_DEPTH_READ;
      initialStates = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else if (myProperties.myIsRenderTarget)
    {
      writeStateMask |= D3D12_RESOURCE_STATE_RENDER_TARGET;
      initialStates = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    // If we can directly initialize the texture, start in the COPY_DST state so we can avoid one barrier
    const bool hasInitData = someInitialDatas != nullptr && aNumInitialDatas > 0u;
    if (hasInitData)
      initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
    
    mySubresources = SubresourceRange(0u, myProperties.myNumMipLevels, 0u, myProperties.GetArraySize(), 0, formatInfo.myNumPlanes);

    GpuSubresourceHazardDataDX12 subHazardData;
    subHazardData.myContext = CommandListType::Graphics;
    subHazardData.myStates = initialStates;
    
    GpuResourceHazardDataDX12* hazardData = &dataDx12.myHazardData;
    *hazardData = GpuResourceHazardDataDX12();
    hazardData->mySubresources.resize(mySubresources.GetNumSubresources(), subHazardData);
    hazardData->myReadStates = readStateMask;
    hazardData->myWriteStates = writeStateMask;
    hazardData->myAllSubresourcesSameStates = true;

    const bool useOptimizeClearValue = (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0u
      || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0u;

    D3D12_CLEAR_VALUE clearValue;
    if (myProperties.bIsDepthStencil)
    {
      clearValue.Format = RenderCore_PlatformDX12::GetDepthStencilViewFormat(resourceDesc.Format);
      clearValue.DepthStencil.Depth = 1.0f;
      clearValue.DepthStencil.Stencil = 0u;
    }
    else
    {
      DXGI_FORMAT typedFormat = RenderCore_PlatformDX12::ResolveFormat(myProperties.myFormat);
      clearValue.Format = typedFormat;
      memset(clearValue.Color, 0, sizeof(clearValue.Color));
    }

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();
    const CpuMemoryAccessType gpuMemAccess = (CpuMemoryAccessType)myProperties.myAccessType;
    const D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device->GetResourceAllocationInfo(0u, 1u, &resourceDesc);

    const GpuMemoryType memoryType = (myProperties.myIsRenderTarget || myProperties.bIsDepthStencil) ? GpuMemoryType::RENDERTARGET : GpuMemoryType::TEXTURE;
    const GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(memoryType, gpuMemAccess, allocInfo.SizeInBytes, (uint) allocInfo.Alignment, myName.c_str());
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, allocInfo.Alignment);
    ASSERT_HRESULT(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialStates, useOptimizeClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&dataDx12.myResource)));
    dataDx12.myGpuMemory = gpuMemory;

    // TODO: Decide between placed and committed resources depending on size and alignment requirements (maybe also expose a preference as a flag to the caller?)
    /*
    D3D12_HEAP_DESC heapDesc{ 0u };
    heapDesc.SizeInBytes = allocInfo.SizeInBytes;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.Alignment = 0;
    heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(gpuMemAccess);
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    CheckD3Dcall(device->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, initialState, useOptimizeClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&storageDx12->myResource)));
    */

    eastl::wstring wName = StringUtil::ToWideString(myName);
    dataDx12.myResource->SetName(wName.c_str());

    myNativeData = dataDx12;

    if (hasInitData)
    {
      InitTextureData(someInitialDatas, aNumInitialDatas);
    }
  }
//---------------------------------------------------------------------------//
  uint64 TextureDX12::GetCopyableFootprints(const SubresourceRange& aSubresourceRange,
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* someFootprintsOut,
    uint* someNumRowsOut,
    uint64* someRowSizesOut) const
  {
    ASSERT(IsValid());
    ASSERT(aSubresourceRange.myFirstMipLevel + aSubresourceRange.myNumMipLevels <= mySubresources.myNumMipLevels);
    ASSERT(aSubresourceRange.myFirstArrayIndex + aSubresourceRange.myNumArrayIndices <= mySubresources.myNumArrayIndices);
    ASSERT(aSubresourceRange.myFirstPlane + aSubresourceRange.myNumPlanes <= mySubresources.myNumPlanes);

    ID3D12Resource* texResource = GetData()->myResource.Get();
    const D3D12_RESOURCE_DESC& texResourceDesc = texResource->GetDesc();

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    const int startSubresourceIndex = GetSubresourceIndex(SubresourceLocation(aSubresourceRange.myFirstMipLevel, aSubresourceRange.myFirstArrayIndex, aSubresourceRange.myFirstPlane));
    const int numSubresources = aSubresourceRange.GetNumSubresources();

    uint64 totalSize;
    device->GetCopyableFootprints(&texResourceDesc, startSubresourceIndex, numSubresources, 0u, someFootprintsOut, someNumRowsOut, someRowSizesOut, &totalSize);
    return totalSize;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Destroy()
  {
    GpuResourceDataDX12* dataDx12 = GetData();
    if (dataDx12 != nullptr)
    {
      dataDx12->myResource.Reset();

      if(dataDx12->myGpuMemory.myHeap != nullptr)
        RenderCore::GetPlatformDX12()->ReleaseGpuMemory(dataDx12->myGpuMemory);
    }

    myNativeData.reset();
    myProperties = TextureProperties();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureViewDX12::TextureViewDX12(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aName)
    : TextureView::TextureView(aTexture, someProperties, aName)
  {
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);
    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    
    eastl::string name = myName.empty() ? aTexture->myName : myName;

    bool success = false;
    GpuResourceViewDataDX12 nativeData;
    myType = GpuResourceViewType::NONE;
    if (someProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        myType = GpuResourceViewType::DSV;
        name.append(" DSV");
        nativeData.myDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, name.c_str());
        success = CreateDSV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
      else
      {
        myType = GpuResourceViewType::RTV;
        name.append(" RTV");
        nativeData.myDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, name.c_str());
        success = CreateRTV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
    }
    else
    {
      const GlobalResourceType globalResourceType = GetGlobalResourceType(someProperties);

      if (someProperties.myIsShaderWritable)
      {
        myType = GpuResourceViewType::UAV;
        name.append(" UAV");
        nativeData.myDescriptor = platformDx12->AllocateShaderVisibleDescriptorForGlobalResource(globalResourceType, name.c_str());
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
        success = CreateUAV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
      else
      {
        myType = GpuResourceViewType::SRV;
        name.append(" SRV");
        nativeData.myDescriptor = platformDx12->AllocateShaderVisibleDescriptorForGlobalResource(globalResourceType, name.c_str());
        myGlobalDescriptorIndex = nativeData.myDescriptor.myGlobalResourceIndex;
        success = CreateSRV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
    }

    ASSERT(success && nativeData.myDescriptor.myCpuHandle.ptr != UINT_MAX && myType != GpuResourceViewType::NONE);

    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numTexMips = texProps.myNumMipLevels;
    const uint numTexArraySlices = texProps.GetArraySize();
    
    myNativeData = nativeData;
    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;
    mySubresourceRange = subresourceRange;

    myCoversAllSubresources = subresourceRange.myNumMipLevels == texProps.myNumMipLevels
      && subresourceRange.myNumArrayIndices == texProps.myDepthOrArraySize
      && subresourceRange.myNumPlanes == DataFormatInfo::GetFormatInfo(texProps.myFormat).myNumPlanes;
  }
//---------------------------------------------------------------------------//
  TextureViewDX12::~TextureViewDX12()
  {
    const GpuResourceViewDataDX12& viewData = eastl::any_cast<const GpuResourceViewDataDX12&>(myNativeData);
    RenderCore::GetPlatformDX12()->FreeDescriptor(viewData.myDescriptor);
  }
//---------------------------------------------------------------------------//
  bool TextureViewDX12::CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);
    DXGI_FORMAT dxgiFormat = RenderCore_PlatformDX12::ResolveFormat(someProperties.myFormat);
    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;
    if (formatInfo.myIsDepthStencil)
    {
      ASSERT(subresourceRange.myFirstPlane <= 1);
      ASSERT(subresourceRange.myNumPlanes == 1u);
      dxgiFormat = subresourceRange.myFirstPlane == 0 ? RenderCore_PlatformDX12::GetDepthViewFormat(dxgiFormat) : RenderCore_PlatformDX12::GetStencilViewFormat(dxgiFormat);
    }

    srvDesc.Format = dxgiFormat;

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
      srvDesc.Texture1D.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.Texture1D.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.Texture1D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
      srvDesc.Texture1DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture1DArray.ArraySize = subresourceRange.myNumArrayIndices;
      srvDesc.Texture1DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      srvDesc.Texture1DArray.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.Texture1DArray.MostDetailedMip = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.PlaneSlice = subresourceRange.myFirstPlane;
      srvDesc.Texture2D.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.Texture2D.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.Texture2D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      srvDesc.Texture2DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture2DArray.ArraySize = subresourceRange.myNumMipLevels;
      srvDesc.Texture2DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      srvDesc.Texture2DArray.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.Texture2DArray.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.Texture2DArray.PlaneSlice = subresourceRange.myFirstPlane;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      srvDesc.Texture3D.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.Texture3D.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.Texture3D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srvDesc.TextureCube.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.TextureCube.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.TextureCube.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
      srvDesc.TextureCubeArray.MipLevels = subresourceRange.myNumMipLevels;
      srvDesc.TextureCubeArray.MostDetailedMip = subresourceRange.myFirstMipLevel;
      srvDesc.TextureCubeArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.TextureCubeArray.First2DArrayFace = subresourceRange.myFirstArrayIndex;
      srvDesc.TextureCubeArray.NumCubes = subresourceRange.myNumArrayIndices;
    }
    else
    {
      ASSERT(false, "Invalid textureView dimension");
      return false;
    }

    GpuResourceDataDX12* dataDx12 = static_cast<const TextureDX12*>(aTexture)->GetData();
    RenderCore::GetPlatformDX12()->GetDevice()->CreateShaderResourceView(dataDx12->myResource.Get(), &srvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool TextureViewDX12::CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = RenderCore::GetPlatformDX12()->ResolveFormat(someProperties.myFormat);
    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;
    ASSERT(subresourceRange.myNumPlanes == 1u);

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
      uavDesc.Texture1D.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
      uavDesc.Texture1DArray.ArraySize = subresourceRange.myNumArrayIndices;
      uavDesc.Texture1DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      uavDesc.Texture1DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Texture2D.MipSlice = subresourceRange.myFirstMipLevel;
      uavDesc.Texture2D.PlaneSlice = subresourceRange.myFirstPlane;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
      uavDesc.Texture2DArray.ArraySize = subresourceRange.myNumArrayIndices;
      uavDesc.Texture2DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      uavDesc.Texture2DArray.PlaneSlice = subresourceRange.myFirstPlane;
      uavDesc.Texture2DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      uavDesc.Texture3D.MipSlice = subresourceRange.myFirstMipLevel;
      uavDesc.Texture3D.FirstWSlice = someProperties.myFirstZindex;
      uavDesc.Texture3D.WSize = someProperties.myZSize;
    }
    else
    {
      ASSERT(false, "Invalid textureView dimension");
      return false;
    }

    GpuResourceDataDX12* dataDx12 = static_cast<const TextureDX12*>(aTexture)->GetData();
    RenderCore::GetPlatformDX12()->GetDevice()->CreateUnorderedAccessView(dataDx12->myResource.Get(), nullptr, &uavDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool TextureViewDX12::CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;

    rtvDesc.Format = RenderCore::GetPlatformDX12()->ResolveFormat(someProperties.myFormat);
    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;
    ASSERT(subresourceRange.myNumPlanes == 1u);

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
      rtvDesc.Texture1D.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
      rtvDesc.Texture1DArray.ArraySize = subresourceRange.myNumArrayIndices;
      rtvDesc.Texture1DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      rtvDesc.Texture1DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtvDesc.Texture2D.MipSlice = subresourceRange.myFirstMipLevel;
      rtvDesc.Texture2D.PlaneSlice = subresourceRange.myFirstPlane;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      rtvDesc.Texture2DArray.ArraySize = subresourceRange.myNumArrayIndices;
      rtvDesc.Texture2DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      rtvDesc.Texture2DArray.PlaneSlice = subresourceRange.myFirstPlane;
      rtvDesc.Texture2DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      rtvDesc.Texture3D.MipSlice = subresourceRange.myFirstMipLevel;
      rtvDesc.Texture3D.FirstWSlice = someProperties.myFirstZindex;
      rtvDesc.Texture3D.WSize = someProperties.myZSize;
    }
    else
    {
      ASSERT(false, "Invalid textureView dimension");
      return false;
    }

    GpuResourceDataDX12* dataDx12 = static_cast<const TextureDX12*>(aTexture)->GetData();
    RenderCore::GetPlatformDX12()->GetDevice()->CreateRenderTargetView(dataDx12->myResource.Get(), &rtvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
  bool TextureViewDX12::CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    const DXGI_FORMAT baseFormat = RenderCore_PlatformDX12::ResolveFormat(someProperties.myFormat);

    const SubresourceRange& subresourceRange = someProperties.mySubresourceRange;

    dsvDesc.Format = RenderCore_PlatformDX12::GetDepthStencilViewFormat(baseFormat);
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    if (someProperties.myIsDepthReadOnly)
      dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    if (someProperties.myIsStencilReadOnly)
      dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
      dsvDesc.Texture1D.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
      dsvDesc.Texture1DArray.ArraySize = subresourceRange.myNumArrayIndices;
      dsvDesc.Texture1DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      dsvDesc.Texture1DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsvDesc.Texture2D.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
      dsvDesc.Texture2DArray.ArraySize = subresourceRange.myNumArrayIndices;
      dsvDesc.Texture2DArray.FirstArraySlice = subresourceRange.myFirstArrayIndex;
      dsvDesc.Texture2DArray.MipSlice = subresourceRange.myFirstMipLevel;
    }
    else
    {
      ASSERT(false, "Invalid textureView dimension");
      return false;
    }

    GpuResourceDataDX12* dataDx12 = static_cast<const TextureDX12*>(aTexture)->GetData();
    RenderCore::GetPlatformDX12()->GetDevice()->CreateDepthStencilView(dataDx12->myResource.Get(), &dsvDesc, aDescriptor.myCpuHandle);
    return true;
  }
//---------------------------------------------------------------------------//
}

#endif