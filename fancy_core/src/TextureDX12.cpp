#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "TextureDX12.h"
#include "Fancy.h"
#include "Renderer.h"
#include "AdapterDX12.h"
#include "DescriptorHeapPoolDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  TextureDX12::TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  TextureDX12::~TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  bool TextureDX12::operator==(const TextureDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  TextureDesc TextureDX12::GetDescription() const
  {
    TextureDesc desc;

    desc.myIsExternalTexture = myParameters.myIsExternalTexture;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    desc.mySourcePath = myParameters.path;

    return desc;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::SetFromDescription(const TextureDesc& aDesc)
  {
    // TODO: Read from binary cache

    myParameters.myIsExternalTexture = aDesc.myIsExternalTexture;
    myParameters.myInternalRefIndex = aDesc.myInternalRefIndex;
    myParameters.path = aDesc.mySourcePath;
  }
//---------------------------------------------------------------------------//
  uint32 locGetBytePerPixelFromFormat(DataFormat aFormat)
  {
    switch(aFormat)
    {
      case DataFormat::R_8UI: 
      case DataFormat::R_8I:
        return 1u;
      case DataFormat::RG_8UI:
      case DataFormat::RG_8I:
      case DataFormat::R_16I:
      case DataFormat::R_16UI:
      case DataFormat::R_16F:
        return 2u;
      case DataFormat::SRGB_8:
      case DataFormat::RGB_8:
      case DataFormat::RGB_8UI:
      case DataFormat::RGB_8I:
        return 3u;
      case DataFormat::SRGB_8_A_8: 
      case DataFormat::RGBA_8:
      case DataFormat::RGBA_8UI:
      case DataFormat::RGBA_8I:
      case DataFormat::RGB_11_11_10F:
      case DataFormat::RG_16F:
      case DataFormat::RG_16UI:
      case DataFormat::RG_16I:
      case DataFormat::R_32F:
      case DataFormat::R_32UI:
      case DataFormat::R_32I:
      case DataFormat::DS_24_8:
        return 4u;
      case DataFormat::RGB_16F:
      case DataFormat::RGB_16UI:
      case DataFormat::RGB_16I:
        return 6u;
      case DataFormat::RGBA_16F: 
      case DataFormat::RGBA_16UI:
      case DataFormat::RGBA_16I:
      case DataFormat::RG_32F:
      case DataFormat::RG_32UI:
      case DataFormat::RG_32I:
        return 8u;
      case DataFormat::RGB_32F:
      case DataFormat::RGB_32UI:
      case DataFormat::RGB_32I:
        return 12u;
      case DataFormat::RGBA_32F: 
      case DataFormat::RGBA_32UI:
      case DataFormat::RGBA_32I:
        return 16u;
      default: 
        ASSERT(false);
        return 0u;
    }
  }
//---------------------------------------------------------------------------//
  void TextureDX12::create(const TextureCreationParams& someParameters, CreationMethod eCreationMethod)
  {
    Destroy();

    RendererDX12* renderer = Fancy::GetRenderer();
    ID3D12Device* device = renderer->GetDevice();

    myParameters = someParameters;

    const bool wantsGpuWriteAccess = someParameters.myIsShaderWritable;

    ASSERT_M(someParameters.u16Width > 0u, "Invalid texture dimension specified");
    ASSERT_M (someParameters.u16Depth == 0u || someParameters.u16Height > 0u, "3D-textures also need a height. Please specify width and height for a 2D texture");
    
    D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    if (someParameters.u16Height > 0u && someParameters.u16Depth == 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    else if (someParameters.u16Height > 0u && someParameters.u16Depth > 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    const uint32 pixelSizeBytes = locGetBytePerPixelFromFormat(someParameters.eFormat);
    uint32 widthSizeBytes = someParameters.u16Width * pixelSizeBytes;
    uint32 heightSizeBytes = 0u;
    uint32 depthSizeBytes = 0u;
    uint32 maxNumMipLevels = 0;

    switch(dimension)
    {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D: 
        heightSizeBytes = 1u;  // Must be 1 per D3D12 reqs
        depthSizeBytes = 0u;  // Interpreted as array-size
        maxNumMipLevels = static_cast<uint32>(glm::log2(someParameters.u16Width));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D: 
        heightSizeBytes = someParameters.u16Height * pixelSizeBytes;
        depthSizeBytes = 0u; // Interpreted as array-size
        maxNumMipLevels = glm::min(glm::log2(someParameters.u16Width), glm::log2(someParameters.u16Height));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D: 
        heightSizeBytes = someParameters.u16Height * pixelSizeBytes;
        depthSizeBytes = someParameters.u16Depth * pixelSizeBytes;
        maxNumMipLevels = glm::min(glm::min(glm::log2(someParameters.u16Width), glm::log2(someParameters.u16Height)), glm::log2(someParameters.u16Depth));
        break;
      default:
        ASSERT(false);
    }

    myState.isCubemap = false;  // TODO: Implement cubemap textures
    myState.isArrayTexture = false;  // TODO: Implement array textures
    myState.isSRGB = (someParameters.eFormat == DataFormat::SRGB_8 || someParameters.eFormat == DataFormat::SRGB_8_A_8);
    if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
      myState.numDimensions = 1u;
    else if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
      myState.numDimensions = 2u;
    else
      myState.numDimensions = 3u;

    uint32 actualNumMipLevels = glm::min(glm::max(1u, static_cast<uint32>(someParameters.u8NumMipLevels)), maxNumMipLevels);
    myParameters.u8NumMipLevels = actualNumMipLevels;

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
    heapProps.CreationNodeMask = 1u;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.VisibleNodeMask = 1u;

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = dimension;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = widthSizeBytes;
    resourceDesc.Height = heightSizeBytes;
    resourceDesc.DepthOrArraySize = depthSizeBytes;
    resourceDesc.MipLevels = actualNumMipLevels;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Format = Adapter::toNativeType(someParameters.eFormat);
    resourceDesc.Flags = wantsGpuWriteAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;
    const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;
    const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;

    myUsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
    if (!wantsCpuWrite && !wantsCpuRead && !wantsCpuStorage)
    {
      // The default for most textures: No Cpu-access at all required. Can be created as GPU-only visible heap
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
      myUsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    else if (wantsCpuWrite || wantsCpuStorage)
    {
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      myUsageState = D3D12_RESOURCE_STATE_GENERIC_READ;

      if (!wantsCpuWrite && !wantsCpuRead)
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
      else if (wantsCpuRead)
        heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
      else  // wants write
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    }
    else if (!wantsCpuWrite && wantsCpuRead)
    {
      heapProps.Type = D3D12_HEAP_TYPE_READBACK;
      heapProps.CPUPageProperty = wantsCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
      myUsageState = D3D12_RESOURCE_STATE_COPY_DEST;
    }

    CheckD3Dcall(renderer->GetDevice()->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      myUsageState,
      nullptr, IID_PPV_ARGS(&myResource)));

    // Create derived views
    DescriptorHeapPoolDX12* heapPool = renderer->GetDescriptorHeapPool();
    DescriptorHeapDX12* heap = heapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = resourceDesc.Format;
    switch(dimension)
    {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D: 
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
          srvDesc.Texture1D.MipLevels = actualNumMipLevels;
          srvDesc.Texture1D.MostDetailedMip = 0u;
          srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        }
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D: 
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srvDesc.Texture2D.MipLevels = actualNumMipLevels;
          srvDesc.Texture2D.MostDetailedMip = 0u;
          srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
          srvDesc.Texture2D.PlaneSlice = 0u;
        }
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
      {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = actualNumMipLevels;
        srvDesc.Texture3D.MostDetailedMip = 0u;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
      }
      break;
    }
    
    mySrvDescriptor = heap->AllocateDescriptor();
    renderer->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptor.GetCpuHandle());

    if (wantsGpuWriteAccess)
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
      uavDesc.Format = resourceDesc.Format;
      switch (dimension)
      {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        {
          uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
          uavDesc.Texture1D.MipSlice = 0u;
        }
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
          uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
          uavDesc.Texture2D.MipSlice = 0u;
          uavDesc.Texture2D.PlaneSlice = 0u;
        }
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        {
          uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
          uavDesc.Texture3D.MipSlice = 0u;
          uavDesc.Texture3D.FirstWSlice = 0u;
          uavDesc.Texture3D.WSize = someParameters.u16Depth;
        }
        break;
      }

      myUavDescriptor = heap->AllocateDescriptor();
      renderer->GetDevice()->CreateUnorderedAccessView(myResource.Get(), nullptr, &uavDesc, myUavDescriptor.GetCpuHandle());
    }

    if (someParameters.myIsRenderTarget)
    {
      D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
      rtvDesc.Format = resourceDesc.Format;
      switch (dimension)
      {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        {
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
          rtvDesc.Texture1D.MipSlice = 0u;
        }
        break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtvDesc.Texture2D.MipSlice = 0u;
          rtvDesc.Texture2D.PlaneSlice = 0u;
        }
        break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        {
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
          rtvDesc.Texture3D.MipSlice = 0u;
          rtvDesc.Texture3D.FirstWSlice = 0u;
          rtvDesc.Texture3D.WSize = someParameters.u16Depth;
        }
        break;
      }
      
      myRtvDescriptor = heap->AllocateDescriptor();
      renderer->GetDevice()->CreateRenderTargetView(myResource.Get(), &rtvDesc, myRtvDescriptor.GetCpuHandle());
    }

    
  }
//---------------------------------------------------------------------------//
  void TextureDX12::setPixelData(void* pData, uint uDataSizeBytes, glm::u32vec3 rectPosOffset, glm::u32vec3 rectDimensions)
  {

  }
//---------------------------------------------------------------------------//
  void* TextureDX12::lock(GpuResoruceLockOption option)
  {
    return nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::unlock()
  {
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Destroy()
  {
    
  }

//---------------------------------------------------------------------------//
} } }

#endif
