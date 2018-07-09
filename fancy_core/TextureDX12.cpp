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
