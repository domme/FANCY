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
#include "DynamicDescriptorHeapDX12.h"
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
    ourCommandQueues[(uint)CommandListType::Graphics].reset(new CommandQueueDX12(CommandListType::Graphics));
    ourCommandQueues[(uint)CommandListType::Compute].reset(new CommandQueueDX12(CommandListType::Compute));

    ourCommandAllocatorPools[(uint)CommandListType::Graphics].reset(new CommandAllocatorPoolDX12(CommandListType::Graphics));
    ourCommandAllocatorPools[(uint)CommandListType::Compute].reset(new CommandAllocatorPoolDX12(CommandListType::Compute));
    
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::CPU_WRITE, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, GpuMemoryAccessType::CPU_READ, 64 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, GpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)GpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, GpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));
    
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 512u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64u));

    return true;
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::Shutdown()
  {
    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      if (ourCommandQueues[i] != nullptr)
        ourCommandQueues[i]->WaitForIdle();

    UpdateAvailableDynamicDescriptorHeaps();
    ASSERT(myAvailableDynamicHeaps.size() == myDynamicHeapPool.size(),
      "There are still some dynamic descriptor heaps in flight when destroying them");
    myAvailableDynamicHeaps.clear();
    myUsedDynamicHeaps.clear();
    myDynamicHeapPool.clear();

    for (uint i = 0u; i < ARRAY_LENGTH(myStaticDescriptorAllocators); ++i)
      myStaticDescriptorAllocators[i].reset();

    for (uint i = 0u; i < (uint)GpuMemoryType::NUM; ++i)
      for (uint k = 0u; k < (uint) GpuMemoryAccessType::NUM; ++k)
        myGpuMemoryAllocators[i][k].reset();

    for (uint i = 0u; i < (uint) CommandListType::NUM; ++i)
      ourCommandAllocatorPools[i].reset();

    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      ourCommandQueues[i].reset();

    ourDevice.Reset();
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12::~RenderCore_PlatformDX12()
  {
    Shutdown();
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
  DescriptorDX12 RenderCore_PlatformDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, const char* aDebugName /* = nullptr*/)
  {
    return myStaticDescriptorAllocators[(uint)aHeapType]->AllocateDescriptor(aDebugName);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseDescriptor(const DescriptorDX12& aDescriptor)
  {
    myStaticDescriptorAllocators[(uint)aDescriptor.myHeapType]->FreeDescriptor(aDescriptor);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::UpdateAvailableDynamicDescriptorHeaps()
  {
    for(auto it = myUsedDynamicHeaps.begin(); it != myUsedDynamicHeaps.end(); )
    {
      uint64 fence = it->first;
      DynamicDescriptorHeapDX12* heap = it->second;

      CommandQueueDX12* queue = (CommandQueueDX12*)GetCommandQueue(CommandQueue::GetCommandListType(fence));
      if (queue->IsFenceDone(fence))
      {
        heap->Reset();
        it = myUsedDynamicHeaps.erase(it);
        myAvailableDynamicHeaps.push_back(heap);
      }
      else
        it++;
    }
  }
//---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12* RenderCore_PlatformDX12::AllocateDynamicDescriptorHeap(uint aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    UpdateAvailableDynamicDescriptorHeaps();
    
    const uint kGpuDescriptorNumIncrement = 16u;
    aDescriptorCount = static_cast<uint>(MathUtil::Align(aDescriptorCount, kGpuDescriptorNumIncrement));

    for (auto it = myAvailableDynamicHeaps.begin(); it != myAvailableDynamicHeaps.end(); ++it)
    {
      DynamicDescriptorHeapDX12* heap = (*it);
      if (heap->myDesc.NumDescriptors == aDescriptorCount && heap->myDesc.Type == aHeapType)
      {
        myAvailableDynamicHeaps.erase(it);
        return heap;
      }
    }
    
    myDynamicHeapPool.push_back(std::make_unique<DynamicDescriptorHeapDX12>(aHeapType, aDescriptorCount));
    return myDynamicHeapPool.back().get();
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseDynamicDescriptorHeap(DynamicDescriptorHeapDX12* aHeap, uint64 aFenceVal)
  {
    myUsedDynamicHeaps.push_back(std::make_pair(aFenceVal, aHeap));
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 RenderCore_PlatformDX12::AllocateGpuMemory(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment, const char* aDebugName /*= nullptr*/)
  {
    return myGpuMemoryAllocators[(uint)aType][(uint)anAccessType]->Allocate(aSize, anAlignment, aDebugName);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation)
  {
    GpuMemoryType type = Adapter::ResolveGpuMemoryType(anAllocation.myHeap->GetDesc().Flags);
    GpuMemoryAccessType accessType = Adapter::ResolveGpuMemoryAccessType(anAllocation.myHeap->GetDesc().Properties.Type);
    myGpuMemoryAllocators[(uint)type][(uint) accessType]->Free(anAllocation);
  }
//---------------------------------------------------------------------------//
  RenderOutput* RenderCore_PlatformDX12::CreateRenderOutput(void* aNativeInstanceHandle, const WindowParameters& someWindowParams)
  {
    return new RenderOutputDX12(aNativeInstanceHandle, someWindowParams);
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
  TextureView* RenderCore_PlatformDX12::CreateTextureView(const SharedPtr<Texture>& aTexture, const TextureViewProperties& someProperties, const char* aDebugName /* = nullptr */)
  {   
    return new TextureViewDX12(aTexture, someProperties);
  }
//---------------------------------------------------------------------------//
  GpuBufferView* RenderCore_PlatformDX12::CreateBufferView(const SharedPtr<GpuBuffer>& aBuffer, const GpuBufferViewProperties& someProperties, const char* aDebugName /* = nullptr */)
  {
    return new GpuBufferViewDX12(aBuffer, someProperties);
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
  DXGI_FORMAT RenderCore_PlatformDX12::GetDepthStencilTextureFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
    {
      // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      return DXGI_FORMAT_R32G8X24_TYPELESS;

      // No Stencil
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
      return DXGI_FORMAT_R32_TYPELESS;

      // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      return DXGI_FORMAT_R24G8_TYPELESS;

      // 16-bit Z w/o Stencil
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
      return DXGI_FORMAT_R16_TYPELESS;

    default:
      return DXGI_FORMAT_UNKNOWN;
    }
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetDepthStencilViewFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
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
  DXGI_FORMAT RenderCore_PlatformDX12::GetDepthViewFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
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
  DXGI_FORMAT RenderCore_PlatformDX12::GetStencilViewFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
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
  DXGI_FORMAT RenderCore_PlatformDX12::GetTypelessFormat(DXGI_FORMAT aFormat)
  {
    switch (aFormat)
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
      return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
    case DXGI_FORMAT_R32G32B32_TYPELESS:
      return DXGI_FORMAT_R32G32B32_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
      return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
      return DXGI_FORMAT_R32G32_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
      return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
      return DXGI_FORMAT_R32G8X24_TYPELESS;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
      // case DXGI_FORMAT_R11G11B10_FLOAT  // This is most likely not a valid format to cast into from DXGI_FORMAT_R10G10B10A2_TYPELESS...
      return DXGI_FORMAT_R10G10B10A2_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
      return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
      return DXGI_FORMAT_R16G16_TYPELESS;
      
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
      return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
      return DXGI_FORMAT_R24G8_TYPELESS;

    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R8G8_TYPELESS:
      return DXGI_FORMAT_R8G8_TYPELESS;

    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_TYPELESS;
        
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_R8_TYPELESS:
        return DXGI_FORMAT_R8_TYPELESS;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC1_TYPELESS:
      return DXGI_FORMAT_BC1_TYPELESS;

    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
      return DXGI_FORMAT_BC2_TYPELESS;

    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
      return DXGI_FORMAT_BC3_TYPELESS;

    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC4_TYPELESS:
      return DXGI_FORMAT_BC4_TYPELESS;
    
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
      return DXGI_FORMAT_BC5_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
      return DXGI_FORMAT_B8G8R8X8_TYPELESS;
        
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC6H_TYPELESS:
      return DXGI_FORMAT_BC6H_TYPELESS;
        
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_BC7_TYPELESS:
      return DXGI_FORMAT_BC7_TYPELESS;

    default:
      ASSERT(false, "Missing typeless format");
      return DXGI_FORMAT_R32G32B32A32_TYPELESS;
    }
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
    case DataFormat::RGB_16: return DataFormat::RGBA_16;
    case DataFormat::RGB_16F: return DataFormat::RGBA_16F;
    case DataFormat::RGB_16UI: return DataFormat::RGBA_16UI;
    case DataFormat::RGB_8UI: return DataFormat::RGBA_8UI;
    default: return aFormat;
    }
  }
//------------------------------------ ---------------------------------------//
  DataFormat RenderCore_PlatformDX12::ResolveFormat(DataFormat aFormat) const
  {
    return locDoResolveFormat(aFormat);
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetDXGIformat(DataFormat aFormat)
  {
    DataFormat supportedFormat = locDoResolveFormat(aFormat);

    switch (supportedFormat)
    {
    case DataFormat::SRGB_8_A_8:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // case DataFormat::SRGB_8:         (Unsupported - DX12 doesn't support 3-component 8 bit formats. Needs to be resolved & padded to 4-component)   
    case DataFormat::RGBA_8:            return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DataFormat::RG_8:              return DXGI_FORMAT_R8G8_UNORM;
    case DataFormat::R_8:               return DXGI_FORMAT_R8_UNORM;
    case DataFormat::RGBA_16:           return DXGI_FORMAT_R16G16B16A16_UNORM;
    //case DataFormat::RGB_16:          (Unsupported - DX12 doesn't support 3-component 8 bit formats. Needs to be resolved & padded to 4-component)   
    case DataFormat::RG_16:             return DXGI_FORMAT_R16G16_UNORM;
    case DataFormat::R_16:              return DXGI_FORMAT_R16_UNORM;
    case DataFormat::RGB_11_11_10F:     return DXGI_FORMAT_R11G11B10_FLOAT;
    case DataFormat::RGBA_16F:          return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case DataFormat::RG_16F:            return DXGI_FORMAT_R16G16_FLOAT;
    case DataFormat::R_16F:             return DXGI_FORMAT_R16_FLOAT;
    case DataFormat::RGBA_32F:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DataFormat::RGB_32F:           return DXGI_FORMAT_R32G32B32_FLOAT;
    case DataFormat::RG_32F:            return DXGI_FORMAT_R32G32_FLOAT;
    case DataFormat::R_32F:             return DXGI_FORMAT_R32_FLOAT;
    case DataFormat::RGBA_32UI:         return DXGI_FORMAT_R32G32B32A32_UINT;
    case DataFormat::RGB_32UI:          return DXGI_FORMAT_R32G32B32_UINT;
    case DataFormat::RG_32UI:           return DXGI_FORMAT_R32G32_UINT;
    case DataFormat::R_32UI:            return DXGI_FORMAT_R32_UINT;
    case DataFormat::RGBA_16UI:         return DXGI_FORMAT_R16G16B16A16_UINT;
    case DataFormat::RG_16UI:           return DXGI_FORMAT_R16G16_UINT;
    case DataFormat::R_16UI:            return DXGI_FORMAT_R16_UINT;
    case DataFormat::RGBA_8UI:          return DXGI_FORMAT_R8G8B8A8_UINT;
    case DataFormat::RG_8UI:            return DXGI_FORMAT_R8G8_UINT;
    case DataFormat::R_8UI:             return DXGI_FORMAT_R8_UINT;
    case DataFormat::D_24UNORM_S_8UI:   return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case DataFormat::UNKNOWN:           return DXGI_FORMAT_UNKNOWN;

    case DataFormat::RGB_8:
    case DataFormat::RGB_16:
    case DataFormat::RGB_16F:
    case DataFormat::RGB_16UI:
    case DataFormat::RGB_8UI:
    default: ASSERT(false, "Missing implementation or unsupported format"); return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
  }
//---------------------------------------------------------------------------//
}
