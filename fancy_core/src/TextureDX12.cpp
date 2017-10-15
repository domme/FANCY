#include "StdAfx.h"
#include "DataFormat.h"

#include "TextureDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

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
  void TextureDX12::Create(const TextureParams& someParameters, const TextureUploadData* someInitialDatas /* = nullptr */, uint32 aNumInitialDatas /*= 0u*/)
  {
    Destroy();

    myParameters = someParameters;

    const bool wantsGpuWriteAccess = someParameters.myIsShaderWritable;

    ASSERT(someParameters.u16Width > 0u, "Invalid texture dimension specified");
    ASSERT(someParameters.u16Depth == 0u || someParameters.u16Height > 0u, "3D-textures also need a height. Please specify width and height for a 2D texture");
    
    D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    if (someParameters.u16Height > 0u && someParameters.u16Depth == 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    else if (someParameters.u16Height > 0u && someParameters.u16Depth > 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    DataFormat actualFormat = RenderCore::ResolveFormat(someParameters.eFormat);
    
    DataFormatInfo formatInfo = DataFormatInfo::GetFormatInfo(actualFormat);
    const uint32 pixelSizeBytes = formatInfo.mySizeBytes;
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

    maxNumMipLevels = glm::max(1u, maxNumMipLevels);

    uint32 depthOrArraySize = glm::max(1u, static_cast<uint32>(someParameters.u16Depth));
    myState.isCubemap = false;  // TODO: Implement cubemap textures
    myState.isArrayTexture = false;  // TODO: Implement array textures
    myState.isSRGB = formatInfo.mySRGB;
    if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
      myState.numDimensions = 1u;
    else if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
      myState.numDimensions = 2u;
    else
      myState.numDimensions = 3u;

    //uint32 actualNumMipLevels = glm::min(glm::max(1u, static_cast<uint32>(someParameters.u8NumMipLevels)), maxNumMipLevels);
    uint32 actualNumMipLevels = 1u; // TODO: Support mipmapping (need a custom compute shader for downsampling)  actualNumMipLevels;
    
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
    resourceDesc.MipLevels = actualNumMipLevels;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Format = RenderCore_PlatformDX12::GetFormat(actualFormat);

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
    if (someParameters.bIsDepthStencil)
    {
      clearValue.Format = RenderCore_PlatformDX12::GetDepthStencilFormat(resourceDesc.Format);
      clearValue.DepthStencil.Depth = 1.0f;
      clearValue.DepthStencil.Stencil = 0u;
    }
    else
    {
      clearValue.Format = resourceDesc.Format;
      memset(clearValue.Color, 0, sizeof(clearValue.Color));
    }

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    
    CheckD3Dcall(platformDx12->GetDevice()->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      myUsageState,
      useOptimizeClearValue ? &clearValue : nullptr,
      IID_PPV_ARGS(&myResource)));

    // Create derived views
    if (someParameters.bIsDepthStencil)
    {
      D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
      dsvDesc.Format = RenderCore_PlatformDX12::GetDepthStencilFormat(resourceDesc.Format);
      dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

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

      myDsvDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
      platformDx12->GetDevice()->CreateDepthStencilView(myResource.Get(), &dsvDesc, myDsvDescriptor.myCpuHandle);

      dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
      myDsvDescriptorReadOnly = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
      platformDx12->GetDevice()->CreateDepthStencilView(myResource.Get(), &dsvDesc, myDsvDescriptor.myCpuHandle);

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = -1;
      srvDesc.Texture2D.MostDetailedMip = 0u;
      srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
      srvDesc.Texture2D.PlaneSlice = 0u;
            
      srvDesc.Format = RenderCore_PlatformDX12::GetDepthFormat(resourceDesc.Format);
      mySrvDescriptorDepth = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      platformDx12->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptorDepth.myCpuHandle);

      srvDesc.Texture2D.PlaneSlice = 1u;
      srvDesc.Format = RenderCore_PlatformDX12::GetStencilFormat(resourceDesc.Format);
      mySrvDescriptorStencil = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      platformDx12->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptorStencil.myCpuHandle);
      
      // Depth-srv is the default srv for DSV textures
      mySrvDescriptor = mySrvDescriptorDepth;
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
      default: { ASSERT(false, "Unsuppoted texture dimension %", dimension); break; }
      }

      mySrvDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      platformDx12->GetDevice()->CreateShaderResourceView(myResource.Get(), &srvDesc, mySrvDescriptor.myCpuHandle);

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
        default: { ASSERT(false, "Unsuppoted texture dimension %", dimension); break; }
        }

        myUavDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        platformDx12->GetDevice()->CreateUnorderedAccessView(myResource.Get(), nullptr, &uavDesc, myUavDescriptor.myCpuHandle);
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
        default: { ASSERT(false, "Unsuppoted texture dimension %", dimension); break; }
        }

        myRtvDescriptor = platformDx12->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        platformDx12->GetDevice()->CreateRenderTargetView(myResource.Get(), &rtvDesc, myRtvDescriptor.myCpuHandle);
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

        TextureUploadData* newDatas = static_cast<TextureUploadData*>(malloc(sizeof(TextureUploadData) * aNumInitialDatas));
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

        RenderCore::InitTextureData(this, newDatas, aNumInitialDatas);

        for (uint32 i = 0u; i < aNumInitialDatas; ++i)
          free(newDatas[i].myData);

        free(newDatas);
      }
      else
      {
        RenderCore::InitTextureData(this, someInitialDatas, aNumInitialDatas);
      }
    }
  }
//---------------------------------------------------------------------------//
  const DescriptorDX12* TextureDX12::GetDescriptor(DescriptorType aType, uint anIndex) const
  {
    switch(aType)
    {
    case DescriptorType::DEFAULT_READ:
      return &mySrvDescriptor;
    case DescriptorType::DEFAULT_READ_DEPTH:
      return &mySrvDescriptorDepth;
    case DescriptorType::DEFAULT_READ_STENCIL:
      return &mySrvDescriptorStencil;
    case DescriptorType::READ_WRITE:
      return  &myUavDescriptor;
    case DescriptorType::RENDER_TARGET:
      return &myRtvDescriptor;
    case DescriptorType::DEPTH_STENCIL:
      return &myDsvDescriptor;
    case DescriptorType::DEPTH_STENCIL_READONLY:
      return &myDsvDescriptorReadOnly;
    }

    return nullptr;
}
//---------------------------------------------------------------------------//
  void TextureDX12::Destroy()
  {
    GpuResourceDX12::Reset();
  }
//---------------------------------------------------------------------------//
} } }
