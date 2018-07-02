#include "StdAfx.h"
#include "DataFormat.h"

#include "TextureDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceStorageDX12.h"
#include "AlignedStorage.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  TextureDX12::TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  TextureDX12::~TextureDX12()
  {
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Create(const TextureParams& someParameters, const TextureSubData* someInitialDatas /* = nullptr */, uint aNumInitialDatas /*= 0u*/)
  {
    Destroy();

    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);

    myParameters = someParameters;
    const bool wantsGpuWriteAccess = someParameters.myIsShaderWritable;

    ASSERT(someParameters.myWidth > 0u, "Invalid texture dimension specified");
    ASSERT(someParameters.myDepthOrArraySize == 0u || someParameters.myHeight > 0u, "3D-textures also need a height. Please specify width and height for a 2D texture");
    
    D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    if (someParameters.myHeight > 0u && someParameters.myDepthOrArraySize == 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    else if (someParameters.myHeight > 0u && someParameters.myDepthOrArraySize > 0u)
      dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    DataFormat actualFormat = RenderCore::ResolveFormat(someParameters.eFormat);
    
    DataFormatInfo formatInfo = DataFormatInfo::GetFormatInfo(actualFormat);
    const uint pixelSizeBytes = formatInfo.mySizeBytes;
    uint maxNumMipLevels = 0;

    switch(dimension)
    {
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D: 
        maxNumMipLevels = static_cast<uint>(glm::log2(someParameters.myWidth));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D: 
        maxNumMipLevels = glm::min(glm::log2(someParameters.myWidth), glm::log2(someParameters.myHeight));
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D: 
        maxNumMipLevels = glm::min(glm::min(glm::log2(someParameters.myWidth), glm::log2(someParameters.myHeight)), glm::log2(someParameters.myDepthOrArraySize));
        break;
      default:
        ASSERT(false);
    }

    maxNumMipLevels = glm::max(1u, maxNumMipLevels);

    uint depthOrArraySize = glm::max(1u, static_cast<uint>(someParameters.myDepthOrArraySize));
    myState.isCubemap = false;  // TODO: Implement cubemap textures
    myState.isArrayTexture = false;  // TODO: Implement array textures
    myState.isSRGB = formatInfo.mySRGB;
    if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
      myState.numDimensions = 1u;
    else if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
      myState.numDimensions = 2u;
    else
      myState.numDimensions = 3u;

    //uint actualNumMipLevels = glm::min(glm::max(1u, static_cast<uint>(someParameters.myNumMipLevels)), maxNumMipLevels);
    uint actualNumMipLevels = 1u; // TODO: Support mipmapping (need a custom compute shader for downsampling)  actualNumMipLevels;
    myParameters.myNumMipLevels = actualNumMipLevels;

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = dimension;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = glm::max(1u, static_cast<uint>(someParameters.myWidth));
    resourceDesc.Height = glm::max(1u, static_cast<uint>(someParameters.myHeight));
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

    myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;

    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someParameters.myAccessType;
    switch (gpuMemAccess)
    {
    case GpuMemoryAccessType::NO_CPU_ACCESS:
      myUsageState = GpuResourceState::RESOURCE_STATE_COMMON;
      break;
    case GpuMemoryAccessType::CPU_WRITE:
      myUsageState = GpuResourceState::RESOURCE_STATE_GENERIC_READ;
      break;
    case GpuMemoryAccessType::CPU_READ:
      myUsageState = GpuResourceState::RESOURCE_STATE_COPY_DEST;
      break;
    }

    const bool useOptimizeClearValue = (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0u
      || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0u;

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

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    D3D12_RESOURCE_STATES usageStateDX12 = Adapter::toNativeType(myUsageState);
    ID3D12Device* device = dx12Platform->GetDevice();

    D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device->GetResourceAllocationInfo(0u, 1u, &resourceDesc);
    
    GpuMemoryType memoryType = (someParameters.myIsRenderTarget || someParameters.bIsDepthStencil) ? GpuMemoryType::RENDERTARGET : GpuMemoryType::TEXTURE;
    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(memoryType, gpuMemAccess, allocInfo.SizeInBytes, allocInfo.Alignment);
    ASSERT(gpuMemory.myHeap != nullptr);

    uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, allocInfo.Alignment);

    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, usageStateDX12, useOptimizeClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&storageDx12->myResource)));

    storageDx12->myGpuMemory = gpuMemory;

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

      myDsvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
      dx12Platform->GetDevice()->CreateDepthStencilView(storageDx12->myResource.Get(), &dsvDesc, myDsvDescriptor.myCpuHandle);

      dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
      myDsvDescriptorReadOnly = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
      dx12Platform->GetDevice()->CreateDepthStencilView(storageDx12->myResource.Get(), &dsvDesc, myDsvDescriptorReadOnly.myCpuHandle);

      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = -1;
      srvDesc.Texture2D.MostDetailedMip = 0u;
      srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
      srvDesc.Texture2D.PlaneSlice = 0u;
            
      srvDesc.Format = RenderCore_PlatformDX12::GetDepthFormat(resourceDesc.Format);
      mySrvDescriptorDepth = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      dx12Platform->GetDevice()->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, mySrvDescriptorDepth.myCpuHandle);

      srvDesc.Texture2D.PlaneSlice = 1u;
      srvDesc.Format = RenderCore_PlatformDX12::GetStencilFormat(resourceDesc.Format);
      mySrvDescriptorStencil = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      dx12Platform->GetDevice()->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, mySrvDescriptorStencil.myCpuHandle);
      
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

      mySrvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
      dx12Platform->GetDevice()->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, mySrvDescriptor.myCpuHandle);

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
          uavDesc.Texture3D.WSize = someParameters.myDepthOrArraySize;
        }
        break;
        default: { ASSERT(false, "Unsuppoted texture dimension %", dimension); break; }
        }

        myUavDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Platform->GetDevice()->CreateUnorderedAccessView(storageDx12->myResource.Get(), nullptr, &uavDesc, myUavDescriptor.myCpuHandle);
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
          rtvDesc.Texture3D.WSize = someParameters.myDepthOrArraySize;
        }
        break;
        default: { ASSERT(false, "Unsuppoted texture dimension %", dimension); break; }
        }

        myRtvDescriptor = dx12Platform->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        dx12Platform->GetDevice()->CreateRenderTargetView(storageDx12->myResource.Get(), &rtvDesc, myRtvDescriptor.myCpuHandle);
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

        TextureSubData* newDatas = static_cast<TextureSubData*>(malloc(sizeof(TextureSubData) * aNumInitialDatas));
        for (uint i = 0u; i < aNumInitialDatas; ++i)
        {
          const uint64 width = someInitialDatas[i].myRowSizeBytes / someInitialDatas[i].myPixelSizeBytes;
          const uint64 height = someInitialDatas[i].mySliceSizeBytes / someInitialDatas[i].myRowSizeBytes;
          const uint64 depth = someInitialDatas[i].myTotalSizeBytes / someInitialDatas[i].mySliceSizeBytes;

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

        RenderCore::UpdateTextureData(this, TextureSubLocation(), newDatas, aNumInitialDatas);

        for (uint i = 0u; i < aNumInitialDatas; ++i)
          free(newDatas[i].myData);

        free(newDatas);
      }
      else
      {
        RenderCore::UpdateTextureData(this, TextureSubLocation(), someInitialDatas, aNumInitialDatas);
      }
    }
  }
//---------------------------------------------------------------------------//
  void TextureDX12::GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const
  {
    // TODO support plane-indices?

    const int arraySize = myState.isArrayTexture ? myParameters.myDepthOrArraySize : 0;

    const int startSubresourceIndex = D3D12CalcSubresource(aStartSubLocation.myMipLevel, aStartSubLocation.myArrayIndex, 0, myParameters.myNumMipLevels, arraySize);

    const int numOverallSubresources = myParameters.myNumMipLevels * glm::max(1, arraySize);

    const int numSubresources = numOverallSubresources - startSubresourceIndex;
    ASSERT(numSubresources > 0);

    ID3D12Resource* texResource = ((GpuResourceStorageDX12*)myStorage.get())->myResource.Get();
    const D3D12_RESOURCE_DESC& texResourceDesc = texResource->GetDesc();

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* placedFootprints = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubresources));
    uint64* rowSizes = static_cast<uint64*>(alloca(sizeof(uint64) * numSubresources));
    uint* numRows = static_cast<uint*>(alloca(sizeof(uint) * numSubresources));

    device->GetCopyableFootprints(&texResourceDesc, startSubresourceIndex, numSubresources, 0u, placedFootprints, numRows, rowSizes, &aTotalSizeOut);

    someLayoutsOut.resize(numSubresources);
    someOffsetsOut.resize(numSubresources);
    for (int i = 0; i < numSubresources; ++i)
    {
      someOffsetsOut[i] = placedFootprints[i].Offset;

      someLayoutsOut[i].myWidth = placedFootprints[i].Footprint.Width;
      someLayoutsOut[i].myHeight = placedFootprints[i].Footprint.Height;
      someLayoutsOut[i].myDepth = placedFootprints[i].Footprint.Depth;
      someLayoutsOut[i].myAlignedRowSize = placedFootprints[i].Footprint.RowPitch;
      someLayoutsOut[i].myRowSize = rowSizes[i];
      someLayoutsOut[i].myNumRows = numRows[i];
      someLayoutsOut[i].myFormat = RenderCore_PlatformDX12::GetFormat(placedFootprints[i].Footprint.Format);
    }
  }
//---------------------------------------------------------------------------//
  uint TextureDX12::GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const
  {
    return
      aSubresourceLocation.myPlaneIndex * myParameters.myNumMipLevels * glm::max(1u, GetArraySize()) +
      aSubresourceLocation.myArrayIndex * myParameters.myNumMipLevels +
      aSubresourceLocation.myMipLevel;
  }
//---------------------------------------------------------------------------//
  TextureSubLocation TextureDX12::GetSubresourceLocation(uint aSubresourceIndex) const
  {
    TextureSubLocation location;
    location.myMipLevel = (aSubresourceIndex % myParameters.myNumMipLevels);
    location.myArrayIndex = ((aSubresourceIndex / myParameters.myNumMipLevels) % glm::max(1u, GetArraySize()));
    location.myPlaneIndex = (aSubresourceIndex / (myParameters.myNumMipLevels * glm::max(1u, GetArraySize())));
    return location;
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
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)myStorage.get();
    if (storageDx12 != nullptr && storageDx12->myGpuMemory.myHeap != nullptr)
    {
      storageDx12->myResource.Reset();
      RenderCore::GetPlatformDX12()->FreeGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
  }
//---------------------------------------------------------------------------//
}
