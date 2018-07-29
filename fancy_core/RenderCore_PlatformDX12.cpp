#include "RenderCore_PlatformDX12.h"
#include "DescriptorDX12.h"
#include "GpuProgramCompilerDX12.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "GpuBufferDX12.h"

#include "MathUtil.h"
#include "GpuProgram.h"
#include "ShaderResourceInterface.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgramCompiler.h"
#include "DescriptorHeapDX12.h"
#include "Window.h"
#include "RenderOutputDX12.h"
#include <malloc.h>
#include "RenderCore.h"
#include "CommandContextDX12.h"
#include "GpuResourceStorageDX12.h"
#include "RenderPlatformCaps.h"
#include "AdapterDX12.h"
#include "GpuResourceViewDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace {
    //---------------------------------------------------------------------------//
    SriResourceType locGetResourceType(D3D12_DESCRIPTOR_RANGE_TYPE aRangeType)
    {
      switch (aRangeType)
      {
      case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: return SriResourceType::BufferOrTexture;
      case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: return SriResourceType::BufferOrTextureRW;
      case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: return SriResourceType::ConstantBuffer;
      case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: return SriResourceType::Sampler;
      default:
        ASSERT(false);
        return SriResourceType::BufferOrTexture;
        break;
      }
    }
  //---------------------------------------------------------------------------//
    
  //---------------------------------------------------------------------------//
    CommandListType locGetCommandListType(D3D12_COMMAND_LIST_TYPE aType)
    {
      switch (aType)
      {
      case D3D12_COMMAND_LIST_TYPE_DIRECT: return CommandListType::Graphics;
      case D3D12_COMMAND_LIST_TYPE_COMPUTE: return CommandListType::Compute;
      case D3D12_COMMAND_LIST_TYPE_COPY: return CommandListType::DMA;
      default:
        ASSERT("Command list type % not supported", aType);
        return CommandListType::Graphics;
      }
    }
  //---------------------------------------------------------------------------//
    std::vector<std::unique_ptr<ShaderResourceInterface>> locShaderResourceInterfacePool;
  }  // namespace
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12::RenderCore_PlatformDX12()
  {
    using namespace Microsoft::WRL;

    memset(ourCommandAllocatorPools, 0u, sizeof(ourCommandAllocatorPools));

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&ourDevice)));
  }
//---------------------------------------------------------------------------//
  bool RenderCore_PlatformDX12::InitInternalResources()
  {
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::CPU_WRITE, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::CPU_READ, 64 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));
    
    ourCommandQueues[(uint)CommandListType::Graphics].reset(new CommandQueueDX12(CommandListType::Graphics));
    ourCommandQueues[(uint)CommandListType::Compute].reset(new CommandQueueDX12(CommandListType::Compute));

    ourCommandAllocatorPools[(uint)CommandListType::Graphics] = new CommandAllocatorPoolDX12(CommandListType::Graphics);
    ourCommandAllocatorPools[(uint)CommandListType::Compute] = new CommandAllocatorPoolDX12(CommandListType::Compute);

    const uint kMaxNumStaticDescriptorsPerHeap = 1000u;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = kMaxNumStaticDescriptorsPerHeap;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0u;

    for (uint i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
      heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);
      ourStaticDescriptorHeaps[i].Create(heapDesc);
    }

    return true;
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12::~RenderCore_PlatformDX12()
  {
    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      ourCommandQueues[i]->WaitForIdle();

    for (uint i = 0u; i < ARRAY_LENGTH(ourCommandAllocatorPools); ++i)
      SAFE_DELETE(ourCommandAllocatorPools[i]);

    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      ourCommandQueues[i].reset();

    ourDevice.Reset();
  }
//---------------------------------------------------------------------------//
  ShaderResourceInterface* RenderCore_PlatformDX12::GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, Microsoft::WRL::ComPtr<ID3D12RootSignature> anRS /* = nullptr */) const
  {
    const uint64 requestedHash = ShaderResourceInterfaceDX12::ComputeHash(anRSdesc);

    for (auto& rs : locShaderResourceInterfacePool)
      if (rs->GetDesc().myHash == requestedHash)
        return rs.get();

    std::unique_ptr<ShaderResourceInterfaceDX12> rs(new ShaderResourceInterfaceDX12());
    if (rs->Create(anRSdesc, anRS))
    {
      ShaderResourceInterface* rs_ptr = rs.get();
      locShaderResourceInterfacePool.push_back(std::move(rs));
      return rs_ptr;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  ID3D12CommandAllocator* RenderCore_PlatformDX12::GetCommandAllocator(CommandListType aCmdListType)
  {
    return ourCommandAllocatorPools[(uint)aCmdListType]->GetNewAllocator();
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, uint64 aFenceVal)
  {
    CommandListType type = CommandQueue::GetCommandListType(aFenceVal);
    ourCommandAllocatorPools[(uint)type]->ReleaseAllocator(anAllocator, aFenceVal);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    return ourStaticDescriptorHeaps[(uint)aHeapType].AllocateDescriptor();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* RenderCore_PlatformDX12::AllocateDynamicDescriptorHeap(uint aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    const uint kGpuDescriptorNumIncrement = 16u;
    aDescriptorCount = static_cast<uint>(MathUtil::Align(aDescriptorCount, kGpuDescriptorNumIncrement));

    auto it = myUsedDynamicHeaps.begin();
    while(it != myUsedDynamicHeaps.end())
    {
      uint64 fence = it->first;
      DescriptorHeapDX12* heap = it->second;

      CommandQueueDX12* queue = (CommandQueueDX12*)GetCommandQueue(CommandQueue::GetCommandListType(fence));
      if (queue->IsFenceDone(fence))
      {
        heap->Reset();
        it = myUsedDynamicHeaps.erase(it);
        if (heap->myDesc.NumDescriptors == aDescriptorCount &&  heap->myDesc.Type == aHeapType)
          return heap;
        else
          myAvailableDynamicHeaps.push_back(heap);
      }
      else
        ++it;
    }

    for (auto it = myAvailableDynamicHeaps.begin(); it != myAvailableDynamicHeaps.end(); ++it)
    {
      DescriptorHeapDX12* heap = (*it);
      if (heap->myDesc.NumDescriptors == aDescriptorCount && heap->myDesc.Type == aHeapType)
      {
        myAvailableDynamicHeaps.erase(it);
        return heap;
      }
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aDescriptorCount;
    heapDesc.Type = aHeapType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;
    myDynamicHeapPool.push_back(std::make_unique<DescriptorHeapDX12>(heapDesc));
    return myDynamicHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, uint64 aFenceVal)
  {
    myUsedDynamicHeaps.push_back(std::make_pair(aFenceVal, aHeap));
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 RenderCore_PlatformDX12::AllocateGpuMemory(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment)
  {
    return myGpuMemoryAllocators[(uint)aType][(uint)anAccessType]->Allocate(aSize, anAlignment);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::FreeGpuMemory(GpuMemoryAllocationDX12& anAllocation)
  {
    GpuMemoryType type = Adapter::ResolveGpuMemoryType(anAllocation.myHeap->GetDesc().Flags);
    GpuMemoryAccessType accessType = Adapter::ResolveGpuMemoryAccessType(anAllocation.myHeap->GetDesc().Properties.Type);
    myGpuMemoryAllocators[(uint)type][(uint) accessType]->Free(anAllocation);
  }
//---------------------------------------------------------------------------//
  RenderOutput* RenderCore_PlatformDX12::CreateRenderOutput(void* aNativeInstanceHandle)
  {
    return new RenderOutputDX12(aNativeInstanceHandle);
  }
//---------------------------------------------------------------------------//
  GpuProgramCompiler* RenderCore_PlatformDX12::CreateShaderCompiler()
  {
    return new GpuProgramCompilerDX12();
  }
//---------------------------------------------------------------------------//
  GpuProgram* RenderCore_PlatformDX12::CreateGpuProgram()
  {
    return new GpuProgramDX12();
  }
//---------------------------------------------------------------------------//
  Texture* RenderCore_PlatformDX12::CreateTexture()
  {
    return new TextureDX12();
  }
//---------------------------------------------------------------------------//
  GpuBuffer* RenderCore_PlatformDX12::CreateBuffer()
  {
   return new GpuBufferDX12();
  }
//---------------------------------------------------------------------------//
  CommandContext* RenderCore_PlatformDX12::CreateContext(CommandListType aType)
  {
    return new CommandContextDX12(aType);
  }
//---------------------------------------------------------------------------//
  TextureView* RenderCore_PlatformDX12::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties)
  {
    DataFormat format = RenderCore::ResolveFormat(someProperties.myFormat);
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);

    GpuResourceViewDataDX12 nativeData;
    nativeData.myType = GpuResourceViewDataDX12::NONE;
    if (someProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        nativeData.myType = GpuResourceViewDataDX12::DSV;
        nativeData.myDescriptor = CreateDSV(aTexture.get(), someProperties);
      }
      else
      {
        nativeData.myType = GpuResourceViewDataDX12::RTV;
        nativeData.myDescriptor = CreateRTV(aTexture.get(), someProperties);
      }
    }
    else
    {
      if (someProperties.myIsShaderWritable)
      {
        nativeData.myType = GpuResourceViewDataDX12::UAV;
        nativeData.myDescriptor = CreateUAV(aTexture.get(), someProperties);
      }
      else
      {
        nativeData.myType = GpuResourceViewDataDX12::SRV;
        nativeData.myDescriptor = CreateSRV(aTexture.get(), someProperties);
      }
    }

    if (nativeData.myDescriptor.myCpuHandle.ptr == 0u || nativeData.myType == GpuResourceViewDataDX12::NONE)
      return nullptr;

    TextureView* textureView = new TextureView(aTexture, someProperties);
    textureView->myNativeData = nativeData;
    return textureView;
  }
//---------------------------------------------------------------------------//
  GpuBufferView* RenderCore_PlatformDX12::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties)
  {
    GpuResourceViewDataDX12 nativeData;
    nativeData.myType = GpuResourceViewDataDX12::NONE;
    if (someProperties.myIsConstantBuffer)
    {
      nativeData.myType = GpuResourceViewDataDX12::CBV;
      nativeData.myDescriptor = CreateCBV(aBuffer.get(), someProperties);
    }
    else if (someProperties.myIsShaderWritable)
    {
      nativeData.myType = GpuResourceViewDataDX12::UAV;
      nativeData.myDescriptor = CreateUAV(aBuffer.get(), someProperties);
    }
    else
    {
      nativeData.myType = GpuResourceViewDataDX12::SRV;
      nativeData.myDescriptor = CreateSRV(aBuffer.get(), someProperties);
    }

    if (nativeData.myDescriptor.myCpuHandle.ptr == 0u || nativeData.myType == GpuResourceViewDataDX12::NONE)
      return nullptr;

    GpuBufferView* textureView = new GpuBufferView(aBuffer, someProperties);
    textureView->myNativeData = nativeData;
    return textureView;
  }
//---------------------------------------------------------------------------//
  Microsoft::WRL::ComPtr<IDXGISwapChain> RenderCore_PlatformDX12::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc)
  {
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC swapChainDesc = aSwapChainDesc;

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(ourCommandQueues[(uint)CommandListType::Graphics]->myQueue.Get(), &swapChainDesc, &swapChain));
    return swapChain;
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::InitCaps()
  {
    myCaps.myMaxNumVertexAttributes = D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT;
    myCaps.myCbufferPlacementAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateSRV(const Texture* aTexture, const TextureViewProperties& someProperties)
  {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = GetFormat(someProperties.myFormat);
    
    if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
      srvDesc.Texture1D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture1D.MostDetailedMip = 0;
      srvDesc.Texture1D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
      srvDesc.Texture1DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture1DArray.ArraySize = someProperties.myArraySize;
      srvDesc.Texture1DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      srvDesc.Texture1DArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture1DArray.MostDetailedMip = 0;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.PlaneSlice = someProperties.myPlaneIndex;
      srvDesc.Texture2D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.Texture2D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      srvDesc.Texture2DArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.Texture2DArray.ArraySize = someProperties.myArraySize;
      srvDesc.Texture2DArray.FirstArraySlice = someProperties.myFirstArrayIndex;
      srvDesc.Texture2DArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture2DArray.MostDetailedMip = 0;
      srvDesc.Texture2DArray.PlaneSlice = someProperties.myPlaneIndex;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_3D)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      srvDesc.Texture3D.MipLevels = someProperties.myNumMipLevels;
      srvDesc.Texture3D.MostDetailedMip = 0;
      srvDesc.Texture3D.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srvDesc.TextureCube.MipLevels = someProperties.myNumMipLevels;
      srvDesc.TextureCube.MostDetailedMip = 0;
      srvDesc.TextureCube.ResourceMinLODClamp = someProperties.myMinLodClamp;
    }
    else if (someProperties.myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY)
    {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
      srvDesc.TextureCubeArray.MipLevels = someProperties.myNumMipLevels;
      srvDesc.TextureCubeArray.MostDetailedMip = 0;
      srvDesc.TextureCubeArray.ResourceMinLODClamp = someProperties.myMinLodClamp;
      srvDesc.TextureCubeArray.First2DArrayFace = someProperties.myFirstArrayIndex;
      srvDesc.TextureCubeArray.NumCubes = someProperties.myArraySize;
    }
    else
    {
      ASSERT(false, "Invalid textureView dimension");
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aTexture->myStorage.get();
    ourDevice->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateSRV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties)
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0u, sizeof(srvDesc));

    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (someProperties.myIsRaw)
    {
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT(someProperties.mySize / 4 <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / 4);
    }
    else if (someProperties.myIsStructured)
    {
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      srvDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT(someProperties.mySize / someProperties.myStructureSize <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / someProperties.myStructureSize);
    }
    else
    {
      DataFormat format = ResolveFormat(someProperties.myFormat);
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      srvDesc.Format = GetFormat(format);
      srvDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      srvDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ourDevice->CreateShaderResourceView(storageDx12->myResource.Get(), &srvDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateUAV(const Texture* aTexture, const TextureViewProperties& someProperties)
  {
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = GetFormat(someProperties.myFormat);

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
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aTexture->myStorage.get();
    ourDevice->CreateUnorderedAccessView(storageDx12->myResource.Get(), nullptr, &uavDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateUAV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties)
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    memset(&uavDesc, 0u, sizeof(uavDesc));

    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    if (someProperties.myIsRaw)
    {
      uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / 4;
      ASSERT(someProperties.mySize / 4 <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / 4);
    }
    else if (someProperties.myIsStructured)
    {
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.StructureByteStride = someProperties.myStructureSize;
      uavDesc.Buffer.FirstElement = someProperties.myOffset / someProperties.myStructureSize;
      ASSERT(someProperties.mySize / someProperties.myStructureSize <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / someProperties.myStructureSize);
    }
    else
    {
      DataFormat format = ResolveFormat(someProperties.myFormat);
      const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
      uavDesc.Format = GetFormat(format);
      uavDesc.Buffer.FirstElement = someProperties.myOffset / formatInfo.mySizeBytes;
      ASSERT(someProperties.mySize / formatInfo.mySizeBytes <= UINT_MAX);
      uavDesc.Buffer.NumElements = static_cast<uint>(someProperties.mySize / formatInfo.mySizeBytes);
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ourDevice->CreateUnorderedAccessView(storageDx12->myResource.Get(), nullptr, &uavDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateRTV(const Texture* aTexture, const TextureViewProperties& someProperties)
  {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = GetFormat(someProperties.myFormat);

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
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aTexture->myStorage.get();
    ourDevice->CreateRenderTargetView(storageDx12->myResource.Get(), &rtvDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateDSV(const Texture* aTexture, const TextureViewProperties& someProperties)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    DXGI_FORMAT baseFormat = GetFormat(someProperties.myFormat);
    dsvDesc.Format = GetDepthStencilFormat(baseFormat);
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
    }

    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aTexture->myStorage.get();
    ourDevice->CreateDepthStencilView(storageDx12->myResource.Get(), &dsvDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::CreateCBV(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someProperties)
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)aBuffer->myStorage.get();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = storageDx12->myResource->GetGPUVirtualAddress() + someProperties.myOffset;
    cbvDesc.SizeInBytes = someProperties.mySize;
    
    DescriptorDX12 descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ourDevice->CreateConstantBufferView(&cbvDesc, descriptor.myCpuHandle);
    return descriptor;
  }
//---------------------------------------------------------------------------//
  D3D12_COMMAND_LIST_TYPE RenderCore_PlatformDX12::GetCommandListType(CommandListType aType)
  {
    switch (aType)
    {
    case CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case CommandListType::DMA: return D3D12_COMMAND_LIST_TYPE_COPY;
    default:
      ASSERT(false);
      return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetDepthStencilFormat(DXGI_FORMAT aDefaultFormat)
  {
    switch (aDefaultFormat)
    {
      // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

      // No Stencil
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
      return DXGI_FORMAT_D32_FLOAT;

      // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      return DXGI_FORMAT_D24_UNORM_S8_UINT;

      // 16-bit Z w/o Stencil
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
      return DXGI_FORMAT_D16_UNORM;

    default:
      return DXGI_FORMAT_UNKNOWN;
    }
  }
  //---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetDepthFormat(DXGI_FORMAT aDefaultFormat)
  {
    switch (aDefaultFormat)
    {
      // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

      // No Stencil
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
      return DXGI_FORMAT_R32_FLOAT;

      // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

      // 16-bit Z w/o Stencil
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
      return DXGI_FORMAT_R16_UNORM;

    default:
      return DXGI_FORMAT_UNKNOWN;
    }
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetStencilFormat(DXGI_FORMAT aDefaultFormat)
  {
    switch (aDefaultFormat)
    {
      // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

      // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

    default:
      return DXGI_FORMAT_UNKNOWN;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_HEAP_TYPE RenderCore_PlatformDX12::ResolveHeapType(GpuMemoryAccessType anAccessType)
  {
    switch (anAccessType) { 
      case GpuMemoryAccessType::NO_CPU_ACCESS: return D3D12_HEAP_TYPE_DEFAULT;
      case GpuMemoryAccessType::CPU_WRITE: return D3D12_HEAP_TYPE_UPLOAD;
      case GpuMemoryAccessType::CPU_READ: return D3D12_HEAP_TYPE_READBACK;
      default: ASSERT(false, "Missing implementation"); return D3D12_HEAP_TYPE_DEFAULT;
    }
  }
//---------------------------------------------------------------------------//
  static DataFormat locDoResolveFormat(DataFormat aFormat)
  {
    switch (aFormat)
    {
    case DataFormat::RGB_8: return DataFormat::RGBA_8;
    case DataFormat::SRGB_8: return DataFormat::SRGB_8_A_8;
    case DataFormat::RGB_16F: return DataFormat::RGBA_16F;
    case DataFormat::RGB_16UI: return DataFormat::RGBA_16UI;
    case DataFormat::RGB_8UI: return DataFormat::RGBA_8UI;
    default: return aFormat;
    }
  }
//------------------------------------ ---------------------------------------//
  DataFormat RenderCore_PlatformDX12::ResolveFormat(DataFormat aFormat)
  {
    return locDoResolveFormat(aFormat);
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetFormat(DataFormat aFormat)
  {
    DataFormat supportedFormat = locDoResolveFormat(aFormat);

    switch (supportedFormat)
    {
    case DataFormat::SRGB_8_A_8:     return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // case DataFormat::SRGB_8:      (Unsupported - DX12 doesn't support 3-component 8 bit formats. Needs to be resolved & padded to 4-component)   
    case DataFormat::RGBA_8:         return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DataFormat::RGB_11_11_10F:  return DXGI_FORMAT_R11G11B10_FLOAT;
    case DataFormat::RGBA_16F:       return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case DataFormat::RG_16F:         return DXGI_FORMAT_R16G16_FLOAT;
    case DataFormat::R_16F:          return DXGI_FORMAT_R16_FLOAT;
    case DataFormat::RGBA_32F:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DataFormat::RGB_32F:        return DXGI_FORMAT_R32G32B32_FLOAT;
    case DataFormat::RG_32F:         return DXGI_FORMAT_R32G32_FLOAT;
    case DataFormat::R_32F:          return DXGI_FORMAT_R32_FLOAT;
    case DataFormat::RGBA_32UI:      return DXGI_FORMAT_R32G32B32A32_UINT;
    case DataFormat::RGB_32UI:       return DXGI_FORMAT_R32G32B32_UINT;
    case DataFormat::RG_32UI:        return DXGI_FORMAT_R32G32_UINT;
    case DataFormat::R_32UI:         return DXGI_FORMAT_R32_UINT;
    case DataFormat::RGBA_16UI:      return DXGI_FORMAT_R16G16B16A16_UINT;
    case DataFormat::RG_16UI:        return DXGI_FORMAT_R16G16_UINT;
    case DataFormat::R_16UI:         return DXGI_FORMAT_R16_UINT;
    case DataFormat::RGBA_8UI:       return DXGI_FORMAT_R8G8B8A8_UINT;
    case DataFormat::RG_8UI:         return DXGI_FORMAT_R8G8_UINT;
    case DataFormat::R_8UI:          return DXGI_FORMAT_R8_UINT;
    case DataFormat::DS_24_8:        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case DataFormat::R_24UNORM_8X:   return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case DataFormat::UNKNOWN:        return DXGI_FORMAT_UNKNOWN;

    case DataFormat::RGB_8:
    case DataFormat::RGB_16F:
    case DataFormat::RGB_16UI:
    case DataFormat::RGB_8UI:
    default: ASSERT(false, "Missing implementation or unsupported format"); return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
  }
//---------------------------------------------------------------------------//
  DataFormat RenderCore_PlatformDX12::GetFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DataFormat::SRGB_8_A_8;
    case DXGI_FORMAT_R8G8B8A8_UNORM:        return DataFormat::RGBA_8;
    case DXGI_FORMAT_R11G11B10_FLOAT:       return DataFormat::RGB_11_11_10F;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:    return DataFormat::RGBA_16F;
    case DXGI_FORMAT_R16G16_FLOAT:          return DataFormat::RG_16F;
    case DXGI_FORMAT_R16_FLOAT:             return DataFormat::R_16F;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:    return DataFormat::RGBA_32F;
    case DXGI_FORMAT_R32G32B32_FLOAT:       return DataFormat::RGB_32F;
    case DXGI_FORMAT_R32G32_FLOAT:          return DataFormat::RG_32F;
    case DXGI_FORMAT_R32_FLOAT:             return DataFormat::R_32F;
    case DXGI_FORMAT_R32G32B32A32_UINT:     return DataFormat::RGBA_32UI;
    case DXGI_FORMAT_R32G32B32_UINT:        return DataFormat::RGB_32UI;
    case DXGI_FORMAT_R32G32_UINT:           return DataFormat::RG_32UI;
    case DXGI_FORMAT_R32_UINT:              return DataFormat::R_32UI;         
    case DXGI_FORMAT_R16G16B16A16_UINT:     return DataFormat::RGBA_16UI;      
    case DXGI_FORMAT_R16G16_UINT:           return DataFormat::RG_16UI;        
    case DXGI_FORMAT_R16_UINT:              return DataFormat::R_16UI;         
    case DXGI_FORMAT_R8G8B8A8_UINT:         return DataFormat::RGBA_8UI;       
    case DXGI_FORMAT_R8G8_UINT:             return DataFormat::RG_8UI;         
    case DXGI_FORMAT_R8_UINT:               return DataFormat::R_8UI;          
    case DXGI_FORMAT_D24_UNORM_S8_UINT:     return DataFormat::DS_24_8;
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return DataFormat::R_24UNORM_8X;
    case DXGI_FORMAT_UNKNOWN:               return DataFormat::UNKNOWN;        
    default: ASSERT(false, "Missing implementation or unsupported format"); return DataFormat::SRGB_8_A_8;
    }
  }
//---------------------------------------------------------------------------//
}
