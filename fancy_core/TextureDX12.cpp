#include "fancy_core_precompile.h"
#include "TextureDX12.h"

#include "DataFormat.h"
#include "RenderCore.h"
#include "StringUtil.h"

#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceViewDX12.h"
#include "CommandList.h"

namespace Fancy {
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
      std::wstring wName = StringUtil::ToWideString(myName);
      dataDx12->myResource->SetName(wName.c_str());
    }
  }
//---------------------------------------------------------------------------//
  GpuResourceDataDX12* TextureDX12::GetData() const
  {
    return myNativeData.IsEmpty() ? nullptr : myNativeData.To<GpuResourceDataDX12*>();
  }
//---------------------------------------------------------------------------//
  uint TextureDX12::CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex)
  {
    return aPlaneIndex * aNumMips * aNumArraySlices +
      anArrayIndex * aNumMips +
      aMipIndex;
  }
//---------------------------------------------------------------------------//
  uint TextureDX12::CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes)
  {
    return aNumMips * aNumArraySlices * aNumPlanes;
  }
//---------------------------------------------------------------------------//
  void TextureDX12::Create(const TextureProperties& someProperties, const char* aName /* = nullptr */, const TextureSubData* someInitialDatas /* = nullptr */, uint aNumInitialDatas /*= 0u*/)
  {
    Destroy();
    GpuResourceDataDX12* dataDx12 = new GpuResourceDataDX12();
    myNativeData = dataDx12;

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
    ASSERT(!someProperties.bIsDepthStencil || !someProperties.myIsRenderTarget, "Render target and depthstencil are mutually exclusive");

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.eFormat);
    ASSERT(formatInfo.myNumComponents != 3, "Texture-formats must be 1-, 2-, or 4-component");
    myProperties.eFormat = someProperties.eFormat;

    myProperties.myDepthOrArraySize = glm::max(1u, myProperties.myDepthOrArraySize);
    
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

    D3D12_RESOURCE_STATES defaultStates = RenderCore_PlatformDX12::ResolveResourceUsageState(someProperties.myDefaultState);
    D3D12_RESOURCE_STATES readStateMask = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
    D3D12_RESOURCE_STATES writeStateMask = D3D12_RESOURCE_STATE_COPY_DEST;
    if (someProperties.myIsShaderWritable)
      writeStateMask |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (someProperties.myIsRenderTarget)
      writeStateMask |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (someProperties.bIsDepthStencil)
    {
      writeStateMask |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
      readStateMask |= D3D12_RESOURCE_STATE_DEPTH_READ;
    }
    
    const uint numArraySlices = myProperties.GetArraySize();
    const uint numSubresources = formatInfo.myNumPlanes * numArraySlices * myProperties.myNumMipLevels;
    myNumSubresources = numSubresources;
    myNumSubresourcesPerPlane = numArraySlices * myProperties.myNumMipLevels;
    myNumPlanes = formatInfo.myNumPlanes;

    myStateTracking = GpuResourceStateTracking();
    myStateTracking.mySubresources.resize(numSubresources);
    myStateTracking.myAllSubresourcesInSameStateAndContext = true;
    for (uint i = 0u; i < numSubresources; ++i)
    {
      myStateTracking.mySubresources[i].myState = someProperties.myDefaultState;
      myStateTracking.mySubresources[i].myQueueType = CommandListType::Graphics;
    }
    myStateTracking.myDx12Data.myReadStates = readStateMask;
    myStateTracking.myDx12Data.myWriteStates = writeStateMask;

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
    const CpuMemoryAccessType gpuMemAccess = (CpuMemoryAccessType)someProperties.myAccessType;
    const D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device->GetResourceAllocationInfo(0u, 1u, &resourceDesc);

    const GpuMemoryType memoryType = (someProperties.myIsRenderTarget || someProperties.bIsDepthStencil) ? GpuMemoryType::RENDERTARGET : GpuMemoryType::TEXTURE;
    const GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(memoryType, gpuMemAccess, allocInfo.SizeInBytes, (uint) allocInfo.Alignment, myName.c_str());
    ASSERT(gpuMemory.myHeap != nullptr);

    const uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, allocInfo.Alignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, defaultStates, useOptimizeClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&dataDx12->myResource)));
    dataDx12->myGpuMemory = gpuMemory;

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

    std::wstring wName = StringUtil::ToWideString(myName);
    dataDx12->myResource->SetName(wName.c_str());

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


        CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics, (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING);
        ctx->ResourceBarrier(this, myProperties.myDefaultState, GpuResourceUsageState::WRITE_COPY_DEST);
        ctx->UpdateTextureData(this, TextureSubLocation(), newDatas, aNumInitialDatas);
        ctx->ResourceBarrier(this, GpuResourceUsageState::WRITE_COPY_DEST, myProperties.myDefaultState);
        RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);

        for (uint i = 0u; i < aNumInitialDatas; ++i)
          free(newDatas[i].myData);

        free(newDatas);
      }
      else
      {
        CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics, (uint) CommandListFlags::NO_RESOURCE_STATE_TRACKING);
        ctx->ResourceBarrier(this, myProperties.myDefaultState, GpuResourceUsageState::WRITE_COPY_DEST);
        ctx->UpdateTextureData(this, TextureSubLocation(), someInitialDatas, aNumInitialDatas);
        ctx->ResourceBarrier(this, GpuResourceUsageState::WRITE_COPY_DEST, myProperties.myDefaultState);
        RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
      }
    }
  }
//---------------------------------------------------------------------------//
  void TextureDX12::GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const
  {
    // TODO support plane-indices?
    ASSERT(IsValid());

    const int startSubresourceIndex = GetSubresourceIndex(aStartSubLocation);

    const uint arraySize = myProperties.GetArraySize();
    const int numOverallSubresources = myProperties.myNumMipLevels * arraySize;
    const int numSubresources = glm::min((int)aNumSubDatas, numOverallSubresources - startSubresourceIndex);
    ASSERT(numSubresources > 0);

    ID3D12Resource* texResource = GetData()->myResource.Get();
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
    const uint index = CalcSubresourceIndex(aSubresourceLocation.myMipLevel, myProperties.myNumMipLevels,
      aSubresourceLocation.myArrayIndex, myProperties.GetArraySize(),
      aSubresourceLocation.myPlaneIndex);

    ASSERT(index < myNumSubresources);
    return index;
  }
//---------------------------------------------------------------------------//
  TextureSubLocation TextureDX12::GetSubresourceLocation(uint aSubresourceIndex) const
  {
    ASSERT(aSubresourceIndex < myNumSubresources);

    TextureSubLocation location;
    location.myMipLevel = aSubresourceIndex % myProperties.myNumMipLevels;
    location.myArrayIndex = (aSubresourceIndex / myProperties.myNumMipLevels) % myProperties.GetArraySize();
    location.myPlaneIndex = aSubresourceIndex / (myProperties.myNumMipLevels * myProperties.GetArraySize());
    return location;
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

      delete dataDx12;
    }

    myNativeData.Clear();
    myStateTracking = GpuResourceStateTracking();
    myProperties = TextureProperties();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureViewDX12::TextureViewDX12(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
    : TextureView::TextureView(aTexture, someProperties)
  {
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);
    
    String name = aTexture->myName;

    bool success = false;
    GpuResourceViewDataDX12 nativeData;
    nativeData.myType = GpuResourceViewDataDX12::NONE;
    if (someProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        nativeData.myType = GpuResourceViewDataDX12::DSV;
        name.append(" DSV");
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, name.c_str());
        success = CreateDSV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
      else
      {
        nativeData.myType = GpuResourceViewDataDX12::RTV;
        name.append(" RTV");
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, name.c_str());
        success = CreateRTV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
    }
    else
    {
      if (someProperties.myIsShaderWritable)
      {
        nativeData.myType = GpuResourceViewDataDX12::UAV;
        name.append(" UAV");
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, name.c_str());
        success = CreateUAV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
      else
      {
        nativeData.myType = GpuResourceViewDataDX12::SRV;
        name.append(" SRV");
        nativeData.myDescriptor = RenderCore::GetPlatformDX12()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, name.c_str());
        success = CreateSRV(aTexture.get(), someProperties, nativeData.myDescriptor);
      }
    }

    ASSERT(success && nativeData.myDescriptor.myCpuHandle.ptr != 0u && nativeData.myType != GpuResourceViewDataDX12::NONE);

    const TextureProperties& texProps = aTexture->GetProperties();
    const uint numTexMips = texProps.myNumMipLevels;
    const uint numTexArraySlices = texProps.GetArraySize();
    
    myNativeData = nativeData;
    mySubresources->reserve(aTexture->myNumSubresources);

    if (nativeData.myType != GpuResourceViewDataDX12::DSV)
    {
      for (uint iArray = someProperties.myFirstArrayIndex; iArray < someProperties.myFirstArrayIndex + someProperties.myArraySize; ++iArray)
        for (uint iMip = someProperties.myMipIndex; iMip < someProperties.myMipIndex + someProperties.myNumMipLevels; ++iMip)
          mySubresources[0].push_back(TextureDX12::CalcSubresourceIndex(iMip, numTexMips, iArray, numTexArraySlices, someProperties.myPlaneIndex));

      myCoversAllSubresources = mySubresources[0].size() == aTexture->myNumSubresources;
    }
    else // DSV
    {
      ASSERT(formatInfo.myNumPlanes <= GpuResourceView::ourNumSupportedPlanes);
      for (int i = 0; i < (int) formatInfo.myNumPlanes; ++i)
      {
        for (uint iArray = someProperties.myFirstArrayIndex; iArray < someProperties.myFirstArrayIndex + someProperties.myArraySize; ++iArray)
          for (uint iMip = someProperties.myMipIndex; iMip < someProperties.myMipIndex + someProperties.myNumMipLevels; ++iMip)
            mySubresources[i].push_back(TextureDX12::CalcSubresourceIndex(iMip, numTexMips, iArray, numTexArraySlices, i));
      }

      myCoversAllSubresources = mySubresources[0].size() == aTexture->myNumSubresourcesPerPlane;
    }
  }
//---------------------------------------------------------------------------//
  TextureViewDX12::~TextureViewDX12()
  {
    const GpuResourceViewDataDX12& viewData = myNativeData.To<GpuResourceViewDataDX12>();
    RenderCore::GetPlatformDX12()->ReleaseDescriptor(viewData.myDescriptor);
  }
//---------------------------------------------------------------------------//
  bool TextureViewDX12::CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties, const DescriptorDX12& aDescriptor)
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(someProperties.myFormat);
    DXGI_FORMAT dxgiFormat = RenderCore_PlatformDX12::GetDXGIformat(someProperties.myFormat);
    if (formatInfo.myIsDepthStencil)
    {
      ASSERT(someProperties.myPlaneIndex <= 1);
      dxgiFormat = someProperties.myPlaneIndex == 0 ? RenderCore_PlatformDX12::GetDepthViewFormat(dxgiFormat) : RenderCore_PlatformDX12::GetStencilViewFormat(dxgiFormat);
    }

    srvDesc.Format = dxgiFormat;

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
      srvDesc.Texture1D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture1D.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.Texture1D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
      srvDesc.Texture1DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture1DArray.ArraySize = someProperties.myArraySize;
      srvDesc.Texture1DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      srvDesc.Texture1DArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture1DArray.MostDetailedMip = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.PlaneSlice = someProperties.myPlaneIndex;
      srvDesc.Texture2D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture2D.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.Texture2D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      srvDesc.Texture2DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture2DArray.ArraySize = someProperties.myArraySize;
      srvDesc.Texture2DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      srvDesc.Texture2DArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture2DArray.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.Texture2DArray.PlaneSlice = someProperties.myPlaneIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      srvDesc.Texture3D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture3D.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.Texture3D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srvDesc.TextureCube.MipLevels = someProperties.myNumMipLevels;
      srvDesc.TextureCube.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.TextureCube.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
      srvDesc.TextureCubeArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.TextureCubeArray.MostDetailedMip = someProperties.myMipIndex;
      srvDesc.TextureCubeArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.TextureCubeArray.First2DArrayFace = someProperties.myFirstArrayIndex;
      srvDesc.TextureCubeArray.NumCubes = someProperties.myArraySize;
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
    uavDesc.Format = RenderCore::GetPlatformDX12()->GetDXGIformat(someProperties.myFormat);

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
      uavDesc.Texture1D.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
      uavDesc.Texture1DArray.ArraySize = someProperties.myArraySize;
      uavDesc.Texture1DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      uavDesc.Texture1DArray.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Texture2D.MipSlice = someProperties.myMipIndex;
      uavDesc.Texture2D.PlaneSlice = someProperties.myPlaneIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
      uavDesc.Texture2DArray.ArraySize = someProperties.myArraySize;
      uavDesc.Texture2DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      uavDesc.Texture2DArray.PlaneSlice = someProperties.myPlaneIndex;
      uavDesc.Texture2DArray.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
      uavDesc.Texture3D.MipSlice = someProperties.myMipIndex;
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

    rtvDesc.Format = RenderCore::GetPlatformDX12()->GetDXGIformat(someProperties.myFormat);

    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
      rtvDesc.Texture1D.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
      rtvDesc.Texture1DArray.ArraySize = someProperties.myArraySize;
      rtvDesc.Texture1DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      rtvDesc.Texture1DArray.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtvDesc.Texture2D.MipSlice = someProperties.myMipIndex;
      rtvDesc.Texture2D.PlaneSlice = someProperties.myPlaneIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      rtvDesc.Texture2DArray.ArraySize = someProperties.myArraySize;
      rtvDesc.Texture2DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      rtvDesc.Texture2DArray.PlaneSlice = someProperties.myPlaneIndex;
      rtvDesc.Texture2DArray.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      rtvDesc.Texture3D.MipSlice = someProperties.myMipIndex;
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
    const DXGI_FORMAT baseFormat = RenderCore_PlatformDX12::GetDXGIformat(someProperties.myFormat);

    dsvDesc.Format = RenderCore_PlatformDX12::GetDepthStencilViewFormat(baseFormat);
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    if (someProperties.myIsDepthReadOnly)
      dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    if (someProperties.myIsStencilReadOnly)
      dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
    
    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
      dsvDesc.Texture1D.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
      dsvDesc.Texture1DArray.ArraySize = someProperties.myArraySize;
      dsvDesc.Texture1DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      dsvDesc.Texture1DArray.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsvDesc.Texture2D.MipSlice = someProperties.myMipIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
      dsvDesc.Texture2DArray.ArraySize = someProperties.myArraySize;
      dsvDesc.Texture2DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      dsvDesc.Texture2DArray.MipSlice = someProperties.myMipIndex;
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
