#include "fancy_core_precompile.h"

#include "RenderCore_PlatformDX12.h"
#include "DescriptorDX12.h"
#include "GpuProgramCompilerDX12.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "GpuBufferDX12.h"

#include "GpuProgram.h"
#include "ShaderResourceInterface.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgramCompiler.h"
#include "DynamicDescriptorHeapDX12.h"
#include "RenderOutputDX12.h"
#include "RenderCore.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"
#include "AdapterDX12.h"
#include "GpuQueryHeapDX12.h"

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
        ASSERT("Command list type %d not supported", (uint) aType);
        return CommandListType::Graphics;
      }
    }
  //---------------------------------------------------------------------------//
    std::vector<std::unique_ptr<ShaderResourceInterface>> locShaderResourceInterfacePool;
  }  // namespace
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12::RenderCore_PlatformDX12()
    : RenderCore_Platform(RenderPlatformType::DX12)
  {
    using namespace Microsoft::WRL;

    memset(ourCommandAllocatorPools, 0u, sizeof(ourCommandAllocatorPools));

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&ourDevice)));

    // CheckD3Dcall(ourDevice->SetStablePowerState(true));

    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(ourDevice->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
      D3D12_MESSAGE_SEVERITY severityIds[] =
      {
        D3D12_MESSAGE_SEVERITY_INFO
      };

      D3D12_MESSAGE_ID denyIds[] =
      {
        D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES
      };

      D3D12_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumSeverities = ARRAYSIZE(severityIds);
      filter.DenyList.pSeverityList = severityIds;
      filter.DenyList.NumIDs = ARRAYSIZE(denyIds);
      filter.DenyList.pIDList = denyIds;

      infoQueue->PushStorageFilter(&filter);
    }

    //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
  }
//---------------------------------------------------------------------------//
  bool RenderCore_PlatformDX12::InitInternalResources()
  {
    ourCommandAllocatorPools[(uint)CommandListType::Graphics].reset(new CommandAllocatorPoolDX12(CommandListType::Graphics));
    ourCommandAllocatorPools[(uint)CommandListType::Compute].reset(new CommandAllocatorPoolDX12(CommandListType::Compute));
    
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::CPU_WRITE, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::BUFFER][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::BUFFER, CpuMemoryAccessType::CPU_READ, 64 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::TEXTURE][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::TEXTURE, CpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));

    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::NO_CPU_ACCESS].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::NO_CPU_ACCESS, 64 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::CPU_WRITE].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::CPU_WRITE, 16 * SIZE_MB));
    myGpuMemoryAllocators[(uint)GpuMemoryType::RENDERTARGET][(uint)CpuMemoryAccessType::CPU_READ].reset(new GpuMemoryAllocatorDX12(GpuMemoryType::RENDERTARGET, CpuMemoryAccessType::CPU_READ, 16 * SIZE_MB));
    
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 512u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64u));
    myStaticDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].reset(new StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64u));

    return true;
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::Shutdown()
  {
    UpdateAvailableDynamicDescriptorHeaps();
    ASSERT(myAvailableDynamicHeaps.size() == myDynamicHeapPool.size(),
      "There are still some dynamic descriptor heaps in flight when destroying them");
    myAvailableDynamicHeaps.clear();
    myUsedDynamicHeaps.clear();
    myDynamicHeapPool.clear();

    for (uint i = 0u; i < ARRAY_LENGTH(myStaticDescriptorAllocators); ++i)
      myStaticDescriptorAllocators[i].reset();

    for (uint i = 0u; i < (uint)GpuMemoryType::NUM; ++i)
      for (uint k = 0u; k < (uint) CpuMemoryAccessType::NUM; ++k)
        myGpuMemoryAllocators[i][k].reset();

    for (uint i = 0u; i < (uint) CommandListType::NUM; ++i)
      ourCommandAllocatorPools[i].reset();

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

      CommandQueueDX12* queue = GetCommandQueueDX12(CommandQueue::GetCommandListType(fence));
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
  CommandQueueDX12* RenderCore_PlatformDX12::GetCommandQueueDX12(CommandListType aCommandListType)
  {
    return static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(aCommandListType));
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
  GpuMemoryAllocationDX12 RenderCore_PlatformDX12::AllocateGpuMemory(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aSize, uint anAlignment, const char* aDebugName /*= nullptr*/)
  {
    return myGpuMemoryAllocators[(uint)aType][(uint)anAccessType]->Allocate(aSize, anAlignment, aDebugName);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseGpuMemory(GpuMemoryAllocationDX12& anAllocation)
  {
    GpuMemoryType type = Adapter::ResolveGpuMemoryType(anAllocation.myHeap->GetDesc().Flags);
    CpuMemoryAccessType accessType = Adapter::ResolveGpuMemoryAccessType(anAllocation.myHeap->GetDesc().Properties.Type);
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
  CommandList* RenderCore_PlatformDX12::CreateContext(CommandListType aType, uint someFlags)
  {
    return new CommandListDX12(aType, someFlags);
  }
//---------------------------------------------------------------------------//
  CommandQueue* RenderCore_PlatformDX12::CreateCommandQueue(CommandListType aType)
  {
    return new CommandQueueDX12(aType);
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
  GpuQueryHeap* RenderCore_PlatformDX12::CreateQueryHeap(GpuQueryType aType, uint aNumQueries)
  {
    return new GpuQueryHeapDX12(aType, aNumQueries);
  }
//---------------------------------------------------------------------------//
  uint RenderCore_PlatformDX12::GetQueryTypeDataSize(GpuQueryType aType)
  {
    switch (aType)
    {
      case GpuQueryType::TIMESTAMP: return sizeof(uint64);
      case GpuQueryType::OCCLUSION: return sizeof(uint64);
      case GpuQueryType::NUM:
      default: ASSERT(false); return sizeof(uint64);
    }
  }
//---------------------------------------------------------------------------//
  float64 RenderCore_PlatformDX12::GetGpuTicksToMsFactor(CommandListType aCommandListType)
  {
    uint64 timestampFrequency = 1u;
    CheckD3Dcall(GetCommandQueueDX12(aCommandListType)->myQueue->GetTimestampFrequency(&timestampFrequency));
    return 1000.0f / timestampFrequency;
  }
//---------------------------------------------------------------------------//
  GpuResourceTransitionInfo RenderCore_PlatformDX12::GetTransitionInfo(const GpuResource* aResource,
    GpuResourceUsageState aSrcState, GpuResourceUsageState aDstState, CommandListType aSrcQueue,
    CommandListType aDstQueue, CommandListType aCurrQueue)
  {
    ID3D12Resource* resourceDx12 = aResource->myNativeData.To<GpuResourceDataDX12*>()->myResource.Get();
    GpuResourceStateTracking& resourceStateTracking = aResource->myStateTracking;
    if (!resourceStateTracking.myCanChangeStates)
      return { };

    const bool srcIsRead = aSrcState >= GpuResourceUsageState::FIRST_READ_STATE && aSrcState <= GpuResourceUsageState::LAST_READ_STATE;
    const bool dstIsRead = aDstState >= GpuResourceUsageState::FIRST_READ_STATE && aDstState <= GpuResourceUsageState::LAST_READ_STATE;

    uint srcStateDx12 = RenderCore_PlatformDX12::ResolveResourceUsageState(aSrcState);
    uint dstStateDx12 = RenderCore_PlatformDX12::ResolveResourceUsageState(aDstState);

    const uint resourceWriteStateMask = resourceStateTracking.myDx12Data.myWriteStates;
    const uint resourceReadStateMask = resourceStateTracking.myDx12Data.myReadStates;
    const bool srcWas0 = srcStateDx12 == 0;
    const bool dstWas0 = dstStateDx12 == 0;

    const uint stateMaskFrom = aSrcQueue == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;
    const uint stateMaskTo = aDstQueue == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;
    const uint wantedSrcStateDx12 = srcStateDx12 & stateMaskFrom & (srcIsRead ? resourceReadStateMask : resourceWriteStateMask);
    const uint wantedDstStateDx12 = dstStateDx12 & stateMaskTo & (dstIsRead ? resourceReadStateMask : resourceWriteStateMask);

    const uint stateMaskCmdList = aCurrQueue == CommandListType::Graphics ? kResourceStateMask_GraphicsContext : kResourceStateMask_ComputeContext;
    srcStateDx12 = wantedSrcStateDx12 & stateMaskCmdList;
    dstStateDx12 = wantedDstStateDx12 & stateMaskCmdList;

    GpuResourceTransitionInfo info;
    info.myCanTransitionFromSrc = srcWas0 || srcStateDx12 != 0;
    info.myCanTransitionToDst = dstWas0 || dstStateDx12 != 0;
    info.myCanFullyTransitionFromSrc = wantedSrcStateDx12 == srcStateDx12;
    info.myCanFullyTransitionToDst = wantedDstStateDx12 == dstStateDx12;
    return info;
  }
//---------------------------------------------------------------------------//
  Microsoft::WRL::ComPtr<IDXGISwapChain> RenderCore_PlatformDX12::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc)
  {
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC swapChainDesc = aSwapChainDesc;

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(GetCommandQueueDX12(CommandListType::Graphics)->myQueue.Get(), &swapChainDesc, &swapChain));
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
  D3D12_HEAP_TYPE RenderCore_PlatformDX12::ResolveHeapType(CpuMemoryAccessType anAccessType)
  {
    switch (anAccessType) { 
      case CpuMemoryAccessType::NO_CPU_ACCESS: return D3D12_HEAP_TYPE_DEFAULT;
      case CpuMemoryAccessType::CPU_WRITE: return D3D12_HEAP_TYPE_UPLOAD;
      case CpuMemoryAccessType::CPU_READ: return D3D12_HEAP_TYPE_READBACK;
      default: ASSERT(false, "Missing implementation"); return D3D12_HEAP_TYPE_DEFAULT;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_RESOURCE_STATES RenderCore_PlatformDX12::ResolveResourceUsageState(GpuResourceUsageState aState)
  {
    switch (aState)
    {
    case GpuResourceUsageState::COMMON:                               return D3D12_RESOURCE_STATE_COMMON;
    case GpuResourceUsageState::READ_INDIRECT_ARGUMENT:               return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    case GpuResourceUsageState::READ_VERTEX_BUFFER:                   return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case GpuResourceUsageState::READ_INDEX_BUFFER:                    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    case GpuResourceUsageState::READ_VERTEX_SHADER_CONSTANT_BUFFER:   return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case GpuResourceUsageState::READ_VERTEX_SHADER_RESOURCE:          return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    case GpuResourceUsageState::READ_PIXEL_SHADER_CONSTANT_BUFFER:    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case GpuResourceUsageState::READ_PIXEL_SHADER_RESOURCE:           return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case GpuResourceUsageState::READ_COMPUTE_SHADER_CONSTANT_BUFFER:  return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case GpuResourceUsageState::READ_COMPUTE_SHADER_RESOURCE:         return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    case GpuResourceUsageState::READ_ANY_SHADER_CONSTANT_BUFFER:      return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case GpuResourceUsageState::READ_ANY_SHADER_RESOURCE:             return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case GpuResourceUsageState::READ_COPY_SOURCE:                     return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case GpuResourceUsageState::READ_ANY_SHADER_ALL_BUT_DEPTH:        return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
    case GpuResourceUsageState::READ_DEPTH:                           return D3D12_RESOURCE_STATE_DEPTH_READ;
    case GpuResourceUsageState::READ_PRESENT:                         return D3D12_RESOURCE_STATE_PRESENT;
    case GpuResourceUsageState::WRITE_VERTEX_SHADER_UAV:              return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case GpuResourceUsageState::WRITE_PIXEL_SHADER_UAV:               return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case GpuResourceUsageState::WRITE_COMPUTE_SHADER_UAV:             return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case GpuResourceUsageState::WRITE_ANY_SHADER_UAV:                 return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case GpuResourceUsageState::WRITE_RENDER_TARGET:                  return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case GpuResourceUsageState::WRITE_COPY_DEST:                      return D3D12_RESOURCE_STATE_COPY_DEST;
    case GpuResourceUsageState::WRITE_DEPTH:                          return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case GpuResourceUsageState::NUM: break;
    default:
      ASSERT(false); return D3D12_RESOURCE_STATE_COMMON;
    }

    return D3D12_RESOURCE_STATE_COMMON;
  }
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCore_PlatformDX12::GetDXGIformat(DataFormat aFormat)
  {
    switch (aFormat)
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

    case DataFormat::RGB_16F:
    case DataFormat::RGB_16UI:
    case DataFormat::RGB_8UI:
    default: ASSERT(false, "Missing implementation or unsupported format"); return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
  }
//---------------------------------------------------------------------------//
}
