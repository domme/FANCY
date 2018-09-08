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
    Destroy();
  }
//---------------------------------------------------------------------------//
  uint TextureDX12::CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex)
  {
    return aPlaneIndex * aNumMips * aNumArraySlices +
      anArrayIndex * aNumMips +
      aMipIndex;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Create(const TextureProperties& someProperties, const TextureSubData* someInitialDatas /* = nullptr */, uint aNumInitialDatas /*= 0u*/)
  {
    Destroy();
    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);
    myProperties = someProperties;

    bool isArray, isCubemap;
    D3D12_RESOURCE_DIMENSION dimension = Adapter::ResolveResourceDimension(someProperties.myDimension, isCubemap, isArray);
    
    ASSERT(dimension != D3D12_RESOURCE_DIMENSION_BUFFER && dimension != D3D12_RESOURCE_DIMENSION_UNKNOWN, "Invalid texture resource dimension");
    ASSERT(!isCubemap || someProperties.myDepthOrArraySize % 6 == 0, "A cubemap needs at least 6 faces");
    ASSERT(someProperties.myWidth > 0u, "Texture needs a nonzero width");
    ASSERT(someProperties.myHeight > 0u || dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D, "Non-1D textures always need a nonzero height" )
    ASSERT(someProperties.myDepthOrArraySize > 0 || (!isArray && dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D), "Array or 3D-texture need a nonzero depthOrArraySize");
    ASSERT(!someProperties.bIsDepthStencil || !someProperties.myIsShaderWritable, "Shader writable and depthstencil are mutually exclusive");
    ASSERT(!someProperties.bIsDepthStencil || !someProperties.myIsRenderTarget, "Render target and depthstencil are mutually exclusive");

    myProperties.eFormat = RenderCore::ResolveFormat(someProperties.eFormat);
    DXGI_FORMAT dxgiFormat = RenderCore_PlatformDX12::GetDXGIformat(myProperties.eFormat);
    if (!someProperties.myPreferTypedFormat)
    {
      if (someProperties.bIsDepthStencil)
        dxgiFormat = RenderCore_PlatformDX12::GetDepthStencilTextureFormat(dxgiFormat);
      else
        dxgiFormat = RenderCore_PlatformDX12::GetTypelessFormat(dxgiFormat);
    }

    const uint minSide = (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D) ? glm::min(someProperties.myWidth, someProperties.myHeight, someProperties.myDepthOrArraySize) : glm::min(someProperties.myWidth, someProperties.myHeight);
    const uint maxNumMipLevels = 1u + static_cast<uint>(glm::floor(glm::log2(minSide)));
    myProperties.myNumMipLevels = glm::max(1u, glm::min(someProperties.myNumMipLevels, maxNumMipLevels));

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = dimension;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = glm::max(1u, static_cast<uint>(someProperties.myWidth));
    resourceDesc.Height = glm::max(1u, static_cast<uint>(someProperties.myHeight));
    resourceDesc.DepthOrArraySize = glm::max(1u, static_cast<uint>(someProperties.myDepthOrArraySize));
    resourceDesc.MipLevels = myProperties.myNumMipLevels;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Format = dxgiFormat;
    
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (someProperties.bIsDepthStencil)
    {
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
      if (someProperties.myIsShaderWritable)
          resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      else if (someProperties.myIsRenderTarget)
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES readState = (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    if (aNumInitialDatas == 0u)
    {
      if (someProperties.myIsShaderWritable)
        initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
      else if (someProperties.myIsRenderTarget)
        initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
      else if (someProperties.bIsDepthStencil)
      {
        initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        readState = D3D12_RESOURCE_STATE_DEPTH_READ;
      }
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myProperties.eFormat);
    const uint numArraySlices = someProperties.myDimension != GpuResourceDimension::TEXTURE_3D ? someProperties.myDepthOrArraySize : 1u;
    const uint numSubresources = formatInfo.myNumPlanes * numArraySlices * myProperties.myNumMipLevels;

    storageDx12->mySubresourceStates.resize(numSubresources);
    storageDx12->mySubresourceContexts.resize(numSubresources);
    for (uint i = 0u; i < numSubresources; ++i)
    {
      storageDx12->mySubresourceStates[i] = initialState;
      storageDx12->mySubresourceContexts[i] = CommandListType::Graphics;
    }
    
    storageDx12->myReadState = readState;

    const bool useOptimizeClearValue = (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0u
      || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0u;

    D3D12_CLEAR_VALUE clearValue;
    if (someProperties.bIsDepthStencil)
    {
      clearValue.Format = RenderCore_PlatformDX12::GetDepthStencilViewFormat(resourceDesc.Format);
      clearValue.DepthStencil.Depth = 1.0f;
      clearValue.DepthStencil.Stencil = 0u;
    }
    else
    {
      clearValue.Format = resourceDesc.Format;
      memset(clearValue.Color, 0, sizeof(clearValue.Color));
    }

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();
    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someProperties.myAccessType;
    const D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device->GetResourceAllocationInfo(0u, 1u, &resourceDesc);
    
    const GpuMemoryType memoryType = (someProperties.myIsRenderTarget || someProperties.bIsDepthStencil) ? GpuMemoryType::RENDERTARGET : GpuMemoryType::TEXTURE;
    const GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(memoryType, gpuMemAccess, allocInfo.SizeInBytes, (uint) allocInfo.Alignment);
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, allocInfo.Alignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialState, useOptimizeClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&storageDx12->myResource)));
    storageDx12->myGpuMemory = gpuMemory;

    // Initialize texture data?
    if (someInitialDatas != nullptr && aNumInitialDatas > 0u)
    {
      const uint pixelSizeBytes = formatInfo.mySizeBytes;

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

    const int arraySize = IsArray() ? myProperties.myDepthOrArraySize : 0;

    const int startSubresourceIndex = D3D12CalcSubresource(aStartSubLocation.myMipLevel, aStartSubLocation.myArrayIndex, 0, myProperties.myNumMipLevels, arraySize);

    const int numOverallSubresources = myProperties.myNumMipLevels * glm::max(1, arraySize);

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
    }
  }
//---------------------------------------------------------------------------//
  uint TextureDX12::GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const
  {
    return CalcSubresourceIndex(aSubresourceLocation.myMipLevel, myProperties.myNumMipLevels, 
      aSubresourceLocation.myArrayIndex, glm::max(1u, GetArraySize()), 
      aSubresourceLocation.myPlaneIndex);
  }
//---------------------------------------------------------------------------//
  TextureSubLocation TextureDX12::GetSubresourceLocation(uint aSubresourceIndex) const
  {
    TextureSubLocation location;
    location.myMipLevel = (aSubresourceIndex % myProperties.myNumMipLevels);
    location.myArrayIndex = ((aSubresourceIndex / myProperties.myNumMipLevels) % glm::max(1u, GetArraySize()));
    location.myPlaneIndex = (aSubresourceIndex / (myProperties.myNumMipLevels * glm::max(1u, GetArraySize())));
    return location;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Destroy()
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)myStorage.get();
    if (storageDx12 != nullptr)
    {
      storageDx12->myResource.Reset();

      if(storageDx12->myGpuMemory.myHeap != nullptr)
        RenderCore::GetPlatformDX12()->FreeGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
    myProperties = TextureProperties();
  }
//---------------------------------------------------------------------------//
}
