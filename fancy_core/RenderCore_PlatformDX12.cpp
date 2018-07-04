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
  GpuResourceViewData* RenderCore_PlatformDX12::CreateTextureViewData(Texture* aTexture, const TextureViewProperties& someProperties)
  {
    ASSERT(!someProperties.myIsShaderWritable || !someProperties.myIsRenderTarget, "UAV and RTV are mutually exclusive");
    
    DataFormat format = RenderCore::ResolveFormat(someProperties.myFormat);
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);

    DescriptorDX12 descriptor;
    GpuResourceViewDataDX12::Type viewType = GpuResourceViewDataDX12::NONE;
    if (someProperties.myIsRenderTarget)
    {
      if (formatInfo.myIsDepthStencil)
      {
        viewType = GpuResourceViewDataDX12::DSV;
        descriptor = CreateDSV(aTexture, someProperties);
      }
      else
      {
        viewType = GpuResourceViewDataDX12::RTV;
        descriptor = CreateRTV(aTexture, someProperties);
      }
    }
    else
    {
      if (someProperties.myIsShaderWritable)
      {
        viewType = GpuResourceViewDataDX12::UAV;
        descriptor = CreateUAV(aTexture, someProperties);
      }
      else
      {
        viewType = GpuResourceViewDataDX12::SRV;
        descriptor = CreateSRV(aTexture, someProperties);
      }
    }

    if (descriptor.myCpuHandle.ptr == 0u || viewType == GpuResourceViewDataDX12::NONE)
      return nullptr;

    GpuResourceViewDataDX12* viewData = new GpuResourceViewDataDX12();

    return new GpuResourceViewDataDX12{}
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
    case DataFormat::DS_24_8:        return DXGI_FORMAT_R24G8_TYPELESS;
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
    case DXGI_FORMAT_R24G8_TYPELESS:        return DataFormat::DS_24_8;        
    case DXGI_FORMAT_UNKNOWN:               return DataFormat::UNKNOWN;        
    default: ASSERT(false, "Missing implementation or unsupported format"); return DataFormat::SRGB_8_A_8;
    }
  }
//---------------------------------------------------------------------------//
}
