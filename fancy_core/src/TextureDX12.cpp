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
    DXGI_FORMAT internalFormat = Adapter::toNativeType(aFormat);

    switch(internalFormat)
    {
      case DXGI_FORMAT_UNKNOWN: 
        ASSERT(false);
        return 0u;
      case DXGI_FORMAT_R32G32B32A32_TYPELESS: 
      case DXGI_FORMAT_R32G32B32A32_FLOAT: 
      case DXGI_FORMAT_R32G32B32A32_UINT: 
      case DXGI_FORMAT_R32G32B32A32_SINT: 
        return 16u;
      case DXGI_FORMAT_R32G32B32_TYPELESS: 
      case DXGI_FORMAT_R32G32B32_FLOAT: 
      case DXGI_FORMAT_R32G32B32_UINT: 
      case DXGI_FORMAT_R32G32B32_SINT: 
      case DXGI_FORMAT_R16G16B16A16_TYPELESS: 
      case DXGI_FORMAT_R16G16B16A16_FLOAT: 
      case DXGI_FORMAT_R16G16B16A16_UNORM: 
      case DXGI_FORMAT_R16G16B16A16_UINT: 
      case DXGI_FORMAT_R16G16B16A16_SNORM: 
      case DXGI_FORMAT_R16G16B16A16_SINT: 
        return 12u;
      case DXGI_FORMAT_R32G32_TYPELESS: 
      case DXGI_FORMAT_R32G32_FLOAT: 
      case DXGI_FORMAT_R32G32_UINT: 
      case DXGI_FORMAT_R32G32_SINT: 
      case DXGI_FORMAT_R32G8X24_TYPELESS: 
      case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: 
      case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: 
      case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: 
        return 8u;
      case DXGI_FORMAT_R10G10B10A2_TYPELESS: 
      case DXGI_FORMAT_R10G10B10A2_UNORM: 
      case DXGI_FORMAT_R10G10B10A2_UINT: 
      case DXGI_FORMAT_R11G11B10_FLOAT: 
      case DXGI_FORMAT_R8G8B8A8_TYPELESS: 
      case DXGI_FORMAT_R8G8B8A8_UNORM: 
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: 
      case DXGI_FORMAT_R8G8B8A8_UINT: 
      case DXGI_FORMAT_R8G8B8A8_SNORM: 
      case DXGI_FORMAT_R8G8B8A8_SINT: 
      case DXGI_FORMAT_R16G16_TYPELESS: 
      case DXGI_FORMAT_R16G16_FLOAT: 
      case DXGI_FORMAT_R16G16_UNORM: 
      case DXGI_FORMAT_R16G16_UINT: 
      case DXGI_FORMAT_R16G16_SNORM: 
      case DXGI_FORMAT_R16G16_SINT: 
      case DXGI_FORMAT_R32_TYPELESS: 
      case DXGI_FORMAT_D32_FLOAT: 
      case DXGI_FORMAT_R32_FLOAT: 
      case DXGI_FORMAT_R32_UINT: 
      case DXGI_FORMAT_R32_SINT: 
      case DXGI_FORMAT_R24G8_TYPELESS: 
      case DXGI_FORMAT_D24_UNORM_S8_UINT: 
      case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: 
      case DXGI_FORMAT_X24_TYPELESS_G8_UINT: 
        return 4u;
      case DXGI_FORMAT_R8G8_TYPELESS: 
      case DXGI_FORMAT_R8G8_UNORM: 
      case DXGI_FORMAT_R8G8_UINT: 
      case DXGI_FORMAT_R8G8_SNORM: 
      case DXGI_FORMAT_R8G8_SINT: 
      case DXGI_FORMAT_R16_TYPELESS: 
      case DXGI_FORMAT_R16_FLOAT: 
      case DXGI_FORMAT_D16_UNORM: 
      case DXGI_FORMAT_R16_UNORM: 
      case DXGI_FORMAT_R16_UINT: 
      case DXGI_FORMAT_R16_SNORM: 
      case DXGI_FORMAT_R16_SINT: 
        return 2u;
      case DXGI_FORMAT_R8_TYPELESS: 
      case DXGI_FORMAT_R8_UNORM: 
      case DXGI_FORMAT_R8_UINT: 
      case DXGI_FORMAT_R8_SNORM: 
      case DXGI_FORMAT_R8_SINT: 
      case DXGI_FORMAT_A8_UNORM: 
        return 1u;

      // TODO: Check sizes of these types
      // case DXGI_FORMAT_R1_UNORM: 
      // case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: 
      // case DXGI_FORMAT_R8G8_B8G8_UNORM: 
      // case DXGI_FORMAT_G8R8_G8B8_UNORM: 
      // case DXGI_FORMAT_BC1_TYPELESS: 
      // case DXGI_FORMAT_BC1_UNORM: 
      // case DXGI_FORMAT_BC1_UNORM_SRGB: 
      // case DXGI_FORMAT_BC2_TYPELESS: 
      // case DXGI_FORMAT_BC2_UNORM: 
      // case DXGI_FORMAT_BC2_UNORM_SRGB: 
      // case DXGI_FORMAT_BC3_TYPELESS: 
      // case DXGI_FORMAT_BC3_UNORM: 
      // case DXGI_FORMAT_BC3_UNORM_SRGB: 
      // case DXGI_FORMAT_BC4_TYPELESS: 
      // case DXGI_FORMAT_BC4_UNORM: 
      // case DXGI_FORMAT_BC4_SNORM: 
      // case DXGI_FORMAT_BC5_TYPELESS: 
      //case DXGI_FORMAT_BC6H_TYPELESS:
      //case DXGI_FORMAT_BC6H_UF16:
      //case DXGI_FORMAT_BC6H_SF16:
      //case DXGI_FORMAT_BC7_TYPELESS:
      //case DXGI_FORMAT_BC7_UNORM:
      //case DXGI_FORMAT_BC7_UNORM_SRGB:
      //case DXGI_FORMAT_AYUV:
      //case DXGI_FORMAT_Y410:
      //case DXGI_FORMAT_Y416:
      //case DXGI_FORMAT_NV12:
      //case DXGI_FORMAT_P010:
      //case DXGI_FORMAT_P016:
      //case DXGI_FORMAT_420_OPAQUE:
      //case DXGI_FORMAT_YUY2:
      //case DXGI_FORMAT_Y210:
      //case DXGI_FORMAT_Y216:
      //case DXGI_FORMAT_NV11:
      //case DXGI_FORMAT_AI44:
      //case DXGI_FORMAT_IA44:
      //case DXGI_FORMAT_P8:
      //case DXGI_FORMAT_A8P8:
      //case DXGI_FORMAT_B4G4R4A4_UNORM:
      //case DXGI_FORMAT_P208:
      //case DXGI_FORMAT_V208:
      //case DXGI_FORMAT_V408:
      //case DXGI_FORMAT_FORCE_UINT:

      case DXGI_FORMAT_BC5_UNORM: 
      case DXGI_FORMAT_BC5_SNORM: 
      case DXGI_FORMAT_B5G6R5_UNORM: 
      case DXGI_FORMAT_B5G5R5A1_UNORM: 
        return 2u;

      case DXGI_FORMAT_B8G8R8A8_UNORM: 
      case DXGI_FORMAT_B8G8R8X8_UNORM: 
      case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
      case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      case DXGI_FORMAT_B8G8R8X8_TYPELESS:
      case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return 4u;

      
      default: 
        ASSERT(false);
        return 0u;
    }

    // Might come in handy at a different place...
    /*switch(aFormat)
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
    */
  }
//---------------------------------------------------------------------------//
  void TextureDX12::create(const TextureParams& someParameters, const TextureUploadData* someInitialDatas /* = nullptr */, uint32 aNumInitialDatas /*= 0u*/)
  {
    Destroy();

    RendererDX12* renderer = Fancy::GetRenderer();
    ID3D12Device* device = renderer->GetDevice();

    myParameters = someParameters;

    const bool wantsGpuWriteAccess = someParameters.myIsShaderWritable;

    ASSERT(someParameters.u16Width > 0u, "Invalid texture dimension specified");
    ASSERT(someParameters.u16Depth == 0u || someParameters.u16Height > 0u, "3D-textures also need a height. Please specify width and height for a 2D texture");
    
    D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    if (someParameters.u16Height > 0u && someParameters.u16Depth == 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    else if (someParameters.u16Height > 0u && someParameters.u16Depth > 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    const uint32 pixelSizeBytes = locGetBytePerPixelFromFormat(someParameters.eFormat);
    uint32 maxNumMipLevels = 0;

    switch(dimension)
    {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D: 
        maxNumMipLevels = static_cast<uint32>(glm::log2(someParameters.u16Width));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D: 
        maxNumMipLevels = glm::min(glm::log2(someParameters.u16Width), glm::log2(someParameters.u16Height));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D: 
        maxNumMipLevels = glm::min(glm::min(glm::log2(someParameters.u16Width), glm::log2(someParameters.u16Height)), glm::log2(someParameters.u16Depth));
        break;
      default:
        ASSERT(false);
    }

    uint32 depthOrArraySize = glm::max(1u, static_cast<uint32>(someParameters.u16Depth));
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
    myParameters.u8NumMipLevels = 1u; // TODO: Support mipmapping (need a custom compute shader for downsampling)  actualNumMipLevels;
    
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
    heapProps.CreationNodeMask = 1u;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.VisibleNodeMask = 1u;

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = dimension;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = glm::max(1u, static_cast<uint32>(someParameters.u16Width));
    resourceDesc.Height = glm::max(1u, static_cast<uint32>(someParameters.u16Height));
    resourceDesc.DepthOrArraySize = depthOrArraySize;
    resourceDesc.MipLevels = myParameters.u8NumMipLevels;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Format = Adapter::toNativeType(someParameters.eFormat);

    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (someParameters.bIsDepthStencil)
    {
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
      if (wantsGpuWriteAccess)
          resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

      if (someParameters.myIsRenderTarget)
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    const bool useOptimizeClearValue = (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0u
      || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0u;

    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;
    const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;
    const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;

    myUsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    if (!wantsCpuWrite && !wantsCpuRead && !wantsCpuStorage)
    {
      // The default for most textures: No Cpu-access at all required. Can be created as GPU-only visible heap
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    }
    else if (wantsCpuWrite || wantsCpuStorage)
    {
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

      if (wantsCpuRead)
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

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = Adapter::toNativeType(someParameters.eFormat);
    if (someParameters.bIsDepthStencil)
    {
      clearValue.DepthStencil.Depth = 1.0f;
      clearValue.DepthStencil.Stencil = 0u;
    }
    else
    {
      memset(clearValue.Color, 0, sizeof(clearValue.Color));
    }
    
    CheckD3Dcall(renderer->GetDevice()->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      myUsageState,
      useOptimizeClearValue ? &clearValue : nullptr,
      IID_PPV_ARGS(&myResource)));

    // Create derived views
    DescriptorHeapPoolDX12* heapPool = renderer->GetDescriptorHeapPool();
    DescriptorHeapDX12* heap = heapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (someParameters.bIsDepthStencil)
    {
      D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
      dsvDesc.Format = resourceDesc.Format;
      dsvDesc.Flags = D3D12_DSV_FLAG_NONE; // TODO: Implement support for readonly depth or stencil

      switch (dimension)
      {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
      {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
        dsvDesc.Texture1D.MipSlice = 0u;
      }
      break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
      {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0u;
      }
      break;
      }

      myDsvDescriptor = heapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->AllocateDescriptor();
      renderer->GetDevice()->CreateDepthStencilView(myResource.Get(), &dsvDesc, myDsvDescriptor.myCpuHandle);
    }
    else
    {
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Format = resourceDesc.Format;

      switch (dimension)
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
      renderer->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptor.myCpuHandle);

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
        renderer->GetDevice()->CreateUnorderedAccessView(myResource.Get(), nullptr, &uavDesc, myUavDescriptor.myCpuHandle);
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
        renderer->GetDevice()->CreateRenderTargetView(myResource.Get(), &rtvDesc, myRtvDescriptor.myCpuHandle);
      }
    }

    // Initialize texture data?
    if (someInitialDatas != nullptr && aNumInitialDatas > 0u)
    {
      if (pixelSizeBytes > someInitialDatas[0].myPixelSizeBytes)
      {
        // The DX12 pixel size is bigger than the size provided by the upload data. 
        // This can happen e.g. if the data is provided as RGB_8 but DX only supports RGBA_8 formats
        // In this case, we need to manually add some padding per pixel so the upload works

        TextureUploadData* newDatas = static_cast<TextureUploadData*>(alloca(sizeof(TextureUploadData) * aNumInitialDatas));
        for (uint32 i = 0u; i < aNumInitialDatas; ++i)
        {
          const uint32 width = someInitialDatas[i].myRowSizeBytes / someInitialDatas[i].myPixelSizeBytes;
          const uint32 height = someInitialDatas[i].mySliceSizeBytes / someInitialDatas[i].myRowSizeBytes;
          const uint32 depth = someInitialDatas[i].myTotalSizeBytes / someInitialDatas[i].mySliceSizeBytes;

          const uint64 requiredSizeBytes = width * height * depth * pixelSizeBytes;
          newDatas[i].myData = (uint8*)malloc(requiredSizeBytes);
          newDatas[i].myTotalSizeBytes = requiredSizeBytes;
          newDatas[i].myPixelSizeBytes = pixelSizeBytes;
          newDatas[i].myRowSizeBytes = width * pixelSizeBytes;
          newDatas[i].mySliceSizeBytes = width * height * pixelSizeBytes;
          
          const uint64 oldPixelSizeBytes = someInitialDatas[i].myPixelSizeBytes;
          const uint64 numPixels = width * height * depth;
          for (uint64 iPixel = 0u; iPixel < numPixels; ++iPixel)
          {
            // Copy the smaller pixel to the new location
            uint8* destPixelDataPtr = newDatas[i].myData + iPixel * pixelSizeBytes;
            uint8* srcPixelDataPtr = someInitialDatas[i].myData + iPixel * oldPixelSizeBytes;
            memcpy(destPixelDataPtr, srcPixelDataPtr, oldPixelSizeBytes);

            // Add the padding value
            const uint kPaddingValue = ~0u;
            memset(destPixelDataPtr + oldPixelSizeBytes, kPaddingValue, pixelSizeBytes - oldPixelSizeBytes);
          }
        }

        RenderContextDX12::InitTextureData(this, newDatas, aNumInitialDatas);

        for (uint32 i = 0u; i < aNumInitialDatas; ++i)
          free(newDatas[i].myData);
      }
      else
      {
        RenderContextDX12::InitTextureData(this, someInitialDatas, aNumInitialDatas);
      }
    }
  }
//---------------------------------------------------------------------------//
  void TextureDX12::setPixelData(void* pData, uint uDataSizeBytes, glm::u32vec3 rectPosOffset, glm::u32vec3 rectDimensions)
  {
    ASSERT(false, "Not implemented");
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
    GpuResourceDX12::Reset();
  }
//---------------------------------------------------------------------------//
} } }

#endif
