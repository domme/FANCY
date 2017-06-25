#include "RenderCore_PlatformDX12.h"
#include "DescriptorDX12.h"
#include "GpuProgramCompilerDX12.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "GpuBufferDX12.h"
#include "ComputeContextDX12.h"

#include "MathUtil.h"
#include "GpuProgram.h"
#include "ShaderResourceInterface.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgramCompiler.h"
#include "DescriptorHeapPoolDX12.h"
#include "RenderContext.h"
#include "RenderWindow.h"
#include "RenderOutputDX12.h"
#include <malloc.h>
#include "RenderCore.h"

namespace Fancy { namespace Rendering { namespace DX12 {
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
  RenderCore_PlatformDX12::RenderCore_PlatformDX12() : 
    ourDynamicDescriptorHeapPool(nullptr)
  {
    using namespace Microsoft::WRL;

    // ComPtr<ID3D12Debug> debugInterface;
    // if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    //   debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&ourDevice)));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CheckD3Dcall(ourDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ourCommandQueues[(uint)CommandListType::Graphics])));

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    CheckD3Dcall(ourDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ourCommandQueues[(uint)CommandListType::Compute])));
  }
//---------------------------------------------------------------------------//
  bool RenderCore_PlatformDX12::InitInternalResources()
  {
    // Create synchronization objects.
    ourCmdListDoneFences[(uint)CommandListType::Graphics].Init("RenderCore_PlatformDX12::GraphicsCommandListFinished");
    ourCmdListDoneFences[(uint)CommandListType::Compute].Init("RenderCore_PlatformDX12::ComputeCommandListFinished");

    ourCommandAllocatorPools[(uint)CommandListType::Graphics] = new CommandAllocatorPoolDX12(CommandListType::Graphics);
    ourCommandAllocatorPools[(uint)CommandListType::Compute] = new CommandAllocatorPoolDX12(CommandListType::Compute);

    ourDynamicDescriptorHeapPool = new DescriptorHeapPoolDX12();

    const uint32 kMaxNumStaticDescriptorsPerHeap = 1000u;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = kMaxNumStaticDescriptorsPerHeap;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0u;

    for (uint32 i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
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
      ourCmdListDoneFences[i].wait();

    for (CommandAllocatorPoolDX12* allocatorPool : ourCommandAllocatorPools)
      delete allocatorPool;

    memset(ourCommandAllocatorPools, 0u, sizeof(ourCommandAllocatorPools));

    delete ourDynamicDescriptorHeapPool;
    ourDynamicDescriptorHeapPool = nullptr;

    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      ourCommandQueues[i].Reset();

    ourDevice.Reset();
  }
//---------------------------------------------------------------------------//
  Rendering::ShaderResourceInterface* RenderCore_PlatformDX12::GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, Microsoft::WRL::ComPtr<ID3D12RootSignature> anRS /* = nullptr */) const
  {
    const uint& requestedHash = ShaderResourceInterfaceDX12::ComputeHash(anRSdesc);

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
  void RenderCore_PlatformDX12::WaitForFence(CommandListType aType)
  {
    FenceDX12& fence = ourCmdListDoneFences[(uint)aType];
    fence.wait();
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::WaitForFence(CommandListType aType, uint64 aFenceVal)
  {
    FenceDX12& fence = ourCmdListDoneFences[(uint)aType];
    ID3D12CommandQueue* cmdQueue = ourCommandQueues[(uint)aType].Get();

    if (fence.IsDone(aFenceVal))
      return;

    if (fence.GetCurrWaitingFenceVal() >= aFenceVal)
    {
      fence.wait();
    }
    else
    {
      fence.signal(cmdQueue, aFenceVal);
      fence.wait();
    }
  }
//---------------------------------------------------------------------------//
  bool RenderCore_PlatformDX12::IsFenceDone(CommandListType aType, uint64 aFenceVal)
  {
    return ourCmdListDoneFences[(uint)aType].IsDone(aFenceVal);
  }
//---------------------------------------------------------------------------//
  uint64 RenderCore_PlatformDX12::ExecuteCommandList(ID3D12CommandList* aCommandList)
  {
    CommandListType type = locGetCommandListType(aCommandList->GetType());

    ID3D12CommandQueue* cmdQueue = ourCommandQueues[(uint)type].Get();
    FenceDX12& cmdListDoneFence = ourCmdListDoneFences[(uint)type];

    cmdListDoneFence.wait();
    cmdQueue->ExecuteCommandLists(1, &aCommandList);
    return cmdListDoneFence.signal(cmdQueue);
  }
//---------------------------------------------------------------------------//
  ID3D12CommandAllocator* RenderCore_PlatformDX12::GetCommandAllocator(CommandListType aCmdListType)
  {
    return ourCommandAllocatorPools[(uint)aCmdListType]->GetNewAllocator();
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, CommandListType aCmdListType, uint64 aFenceVal)
  {
    ourCommandAllocatorPools[(uint)aCmdListType]->ReleaseAllocator(anAllocator, aFenceVal);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 RenderCore_PlatformDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    return ourStaticDescriptorHeaps[(uint32)aHeapType].AllocateDescriptor();
  }
//---------------------------------------------------------------------------//
  DescriptorHeapDX12* RenderCore_PlatformDX12::AllocateDynamicDescriptorHeap(uint32 aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    return ourDynamicDescriptorHeapPool->AllocateDynamicHeap(aDescriptorCount, aHeapType);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, CommandListType aCmdListType, uint64 aFenceVal)
  {
    ourDynamicDescriptorHeapPool->ReleaseDynamicHeap(aCmdListType, aFenceVal, aHeap);
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
  GpuBuffer* RenderCore_PlatformDX12::CreateGpuBuffer()
  {
    return new GpuBufferDX12();
  }
//---------------------------------------------------------------------------//
  CommandContext* RenderCore_PlatformDX12::CreateContext(CommandListType aType)
  {
    switch(aType)
    {
      case CommandListType::Graphics:
        return new RenderContextDX12();
      case CommandListType::Compute: 
        return new ComputeContextDX12();
      default: return nullptr;
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::InitBufferData(GpuBuffer* aBuffer, void* aDataPtr, CommandContext* aContext)
  {
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    GpuBufferDX12* bufferDx12 = static_cast<GpuBufferDX12*>(aBuffer);

    const D3D12_RESOURCE_DESC& resourceDesc = bufferDx12->GetResource()->GetDesc();

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
    CheckD3Dcall(GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* mappedBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &mappedBufferPtr));
    memcpy(mappedBufferPtr, aDataPtr, aBuffer->GetSizeBytes());
    uploadResource->Unmap(0, nullptr);

    ASSERT(aContext->GetType() == CommandListType::Graphics);
    RenderContextDX12* context = static_cast<RenderContextDX12*>(aContext);

    context->TransitionResource(bufferDx12, D3D12_RESOURCE_STATE_COPY_DEST, true);
    context->myCommandList->CopyResource(bufferDx12->GetResource(), uploadResource.Get());
    context->TransitionResource(bufferDx12, D3D12_RESOURCE_STATE_GENERIC_READ, true);
    context->ExecuteAndReset(true);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::UpdateBufferData(GpuBuffer* aBuffer, void* aDataPtr, uint32 aByteOffset, uint32 aByteSize, CommandContext* aContext)
  {
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    GpuBufferDX12* bufferDx12 = static_cast<GpuBufferDX12*>(aBuffer);

    D3D12_RESOURCE_DESC uploadResourceDesc = bufferDx12->GetResource()->GetDesc();
    uploadResourceDesc.Width = MathUtil::Align(aByteSize, aBuffer->GetAlignment());

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
    CheckD3Dcall(ourDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* uploadBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &uploadBufferPtr));
    memcpy(uploadBufferPtr, aDataPtr, aByteSize);
    uploadResource->Unmap(0, nullptr);

    ASSERT(aContext->GetType() == CommandListType::Graphics);
    RenderContextDX12* context = static_cast<RenderContextDX12*>(aContext);
    context->TransitionResource(bufferDx12, D3D12_RESOURCE_STATE_COPY_DEST, true);
    context->myCommandList->CopyBufferRegion(bufferDx12->GetResource(), aByteOffset, uploadResource.Get(), 0u, aByteSize);
    context->TransitionResource(bufferDx12, D3D12_RESOURCE_STATE_GENERIC_READ, true);
    context->ExecuteAndReset(true);
  }
//---------------------------------------------------------------------------//
  void RenderCore_PlatformDX12::InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas, CommandContext* aContext)
  {
    TextureDX12* textureDx12 = static_cast<TextureDX12*>(aTexture);

    ID3D12Device* device = GetDevice();
    const D3D12_RESOURCE_DESC& resourceDesc = textureDx12->GetResource()->GetDesc();

    // DEBUG: layouts and row-infos not needed here yet
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumUploadDatas));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumUploadDatas));
    uint32* destRowNums = static_cast<uint32*>(alloca(sizeof(uint32) * aNumUploadDatas));

    uint64 requiredStagingBufferSize;
    //device->GetCopyableFootprints(&resourceDesc, 0u, aNumUploadDatas, 0u, nullptr, nullptr, nullptr, &requiredStagingBufferSize);
    device->GetCopyableFootprints(&resourceDesc, 0u, aNumUploadDatas, 0u, destLayouts, destRowNums, destRowSizesByte, &requiredStagingBufferSize);

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC BufferDesc;
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = requiredStagingBufferSize;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12Resource> stagingBuffer;

    CheckD3Dcall(device->CreateCommittedResource(
      &HeapProps, D3D12_HEAP_FLAG_NONE,
      &BufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&stagingBuffer)));

    ASSERT(aTexture->GetParameters().u16Depth <= 1u, "The code below might not work for 3D textures");

    D3D12_SUBRESOURCE_DATA* subDatas = static_cast<D3D12_SUBRESOURCE_DATA*>(alloca(sizeof(D3D12_SUBRESOURCE_DATA) * aNumUploadDatas));
    for (uint32 i = 0u; i < aNumUploadDatas; ++i)
    {
      subDatas[i].pData = someUploadDatas[i].myData;
      subDatas[i].SlicePitch = someUploadDatas[i].mySliceSizeBytes;
      subDatas[i].RowPitch = someUploadDatas[i].myRowSizeBytes;
    }

    ASSERT(aContext->GetType() == CommandListType::Graphics);
    RenderContextDX12* context = static_cast<RenderContextDX12*>(aContext);

    D3D12_RESOURCE_STATES oldUsageState = textureDx12->GetUsageState();
    context->TransitionResource(textureDx12, D3D12_RESOURCE_STATE_COPY_DEST, true);
    context->UpdateSubresources(textureDx12->GetResource(), stagingBuffer.Get(), 0u, aNumUploadDatas, subDatas);
    context->TransitionResource(textureDx12, oldUsageState, true);
    context->ExecuteAndReset(true);
  }
//---------------------------------------------------------------------------//
  Microsoft::WRL::ComPtr<IDXGISwapChain> RenderCore_PlatformDX12::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC& aSwapChainDesc)
  {
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC swapChainDesc = aSwapChainDesc;

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(ourCommandQueues[(uint)CommandListType::Graphics].Get(), &swapChainDesc, &swapChain));
    return swapChain;
  }
//---------------------------------------------------------------------------//
  DataFormatInfo RenderCore_PlatformDX12::GetFormatInfo(DXGI_FORMAT aFormat)
  {
    DataFormatInfo format;
    return format;

    /*
    switch (aFormat)
    {
    case DXGI_FORMAT_UNKNOWN:                   return DataFormatInfo(DataFormat::UNKNOWN, 0u, 0u, false, false, false);
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:     return DataFormatInfo(DataFormat::UNKNOWN, 0u, 0u, false, false, false);
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
    */
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
  static DataFormat locDoResolveFormat(DataFormat aFormat)
  {
    switch (aFormat)
    {
    case DataFormat::RGB_8: return DataFormat::RGBA_8;
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
    case DataFormat::RGBA_8:         return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DataFormat::SRGB_8:         return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
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
    case DataFormat::UNKNOWN:        return DXGI_FORMAT_UNKNOWN;

    case DataFormat::RGB_8:
    case DataFormat::RGB_16F:
    case DataFormat::RGB_16UI:
    case DataFormat::RGB_8UI:
    default: ASSERT(false, "Missing implementation or unsupported format"); return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::DX12