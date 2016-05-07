#include "RendererDX12.h"
#include "AdapterDX12.h"
#include "Fancy.h"


#if defined (RENDERER_DX12)
#include "MathUtil.h"
#include "GpuProgram.h"
#include "ShaderResourceInterface.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgramCompiler.h"
#include "DescriptorHeapPoolDX12.h"
#include "Renderer.h"
#include "RenderContext.h"

namespace Fancy { namespace Rendering { namespace DX12 { 

//---------------------------------------------------------------------------//
  RenderOutputDX12::RenderOutputDX12(void* aNativeWindowHandle)
	{
    CreateSwapChain(aNativeWindowHandle);
	}
//---------------------------------------------------------------------------//
	RenderOutputDX12::~RenderOutputDX12()
	{
	}
//---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateSwapChain(void* aNativeWindowHandle)
  {
    using namespace Microsoft::WRL;

    HWND windowHandle = *static_cast<HWND*>(aNativeWindowHandle);

    ComPtr<IDXGIFactory4> dxgiFactory;
    CheckD3Dcall(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = kBackbufferCount;
    swapChainDesc.BufferDesc.Width = 1280;
    swapChainDesc.BufferDesc.Height = 720;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain> swapChain;
    CheckD3Dcall(dxgiFactory->CreateSwapChain(RenderCore::ourGraphicsCommandQueue.Get(), &swapChainDesc, &swapChain));
    CheckD3Dcall(swapChain.As(&mySwapChain));
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::CreateBackbufferResources()
  {
    DescriptorHeapDX12* rtvHeapCpu = myDescriptorHeapPool->GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    TextureParams dsTexParams;
    dsTexParams.bIsDepthStencil = true;
    dsTexParams.eFormat = DataFormat::DS_24_8;
    dsTexParams.myIsExternalTexture = false;
    dsTexParams.myIsRenderTarget = false;
    dsTexParams.myIsShaderWritable = false;
    dsTexParams.u16Width = 1280u;
    dsTexParams.u16Height = 720u;
    dsTexParams.u8NumMipLevels = 1u;
    myDefaultDepthStencil.create(dsTexParams);

    for (UINT n = 0; n < kBackbufferCount; n++)
    {
      TextureDX12& backbufferResource = myBackbuffers[n];
      backbufferResource.myUsageState = D3D12_RESOURCE_STATE_PRESENT;

      // TODO: Sync this better with swap chain properties
      backbufferResource.myParameters.myIsRenderTarget = true;
      backbufferResource.myParameters.eFormat = DataFormat::RGBA_8;
      backbufferResource.myParameters.u16Width = 1280;
      backbufferResource.myParameters.u16Height = 720;
      backbufferResource.myParameters.u16Depth = 1u;

      CheckD3Dcall(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&backbufferResource.myResource)));
      backbufferResource.myRtvDescriptor = rtvHeapCpu->AllocateDescriptor();
      RenderCoreDX12::ourDevice->CreateRenderTargetView(backbufferResource.myResource.Get(), nullptr, backbufferResource.myRtvDescriptor.myCpuHandle);
    }
  }
//---------------------------------------------------------------------------//
  void RenderOutputDX12::postInit()
	{
    CreateBackbufferResources();
	}
//---------------------------------------------------------------------------//
  void RenderOutputDX12::beginFrame()
	{
    RenderCoreDX12::ourGraphicsFence.wait();  // Needed?
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
	}
//---------------------------------------------------------------------------//
	void RenderOutputDX12::endFrame()
	{
    TextureDX12& currBackbuffer = myBackbuffers[myCurrBackbufferIndex];

    RenderContext* context = RenderContext::AllocateContext();
    context->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_PRESENT, true);
    context->ExecuteAndReset(false);
    RenderContext::FreeContext(context);

    mySwapChain->Present(1, 0);
	}
//---------------------------------------------------------------------------//


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
    ShaderResourceInterfaceDesc locGetInterfaceDescFromRSdesc(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc)
    {
      ShaderResourceInterfaceDesc sri;

      for (uint iParam = 0u; iParam < anRSdesc.NumParameters; ++iParam)
      {
        SriElement sriElement;

        const D3D12_ROOT_PARAMETER& param = anRSdesc.pParameters[iParam];
        switch (param.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
          sriElement.myType = SriElementType::DescriptorSet;
          sriElement.myDescriptorSet.myNumElements = param.DescriptorTable.NumDescriptorRanges;
          ASSERT(param.DescriptorTable.NumDescriptorRanges <= kMaxNumDescriptorSetElements);

          for (uint iRange = 0u; iRange < param.DescriptorTable.NumDescriptorRanges; ++iRange)
          {
            const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[iRange];
            SriDescriptorSetElement& setElement = sriElement.myDescriptorSet.myRangeElements[iRange];
            setElement.myResourceType = locGetResourceType(range.RangeType);
            setElement.myNumElements = range.NumDescriptors;
            setElement.myBindingSlot = range.BaseShaderRegister;
          }
          break;
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
          sriElement.myType = SriElementType::Constants;
          sriElement.myConstants.myBindingSlot = param.Constants.ShaderRegister;
          sriElement.myConstants.myNumValues = param.Constants.Num32BitValues;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::ConstantBuffer;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::BufferOrTexture;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
          sriElement.myType = SriElementType::Descriptor;
          sriElement.myDescriptor.myResourceType = SriResourceType::BufferOrTextureRW;
          sriElement.myDescriptor.myBindingSlot = param.Descriptor.ShaderRegister;
          break;
        default: break;
        }

        sri.myElements.push_back(sriElement);
      }

      return sri;
    }
    //---------------------------------------------------------------------------//
    uint locComputeRSdescHash(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc)
    {
      uint hash = 0u;
      MathUtil::hash_combine(hash, anRSdesc.NumParameters);
      for (uint i = 0u; i < anRSdesc.NumParameters; ++i)
        MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pParameters[i]));

      MathUtil::hash_combine(hash, anRSdesc.NumStaticSamplers);
      for (uint i = 0u; i < anRSdesc.NumStaticSamplers; ++i)
        MathUtil::hash_combine(hash, MathUtil::hashFromGeneric(anRSdesc.pStaticSamplers[i]));

      MathUtil::hash_combine(hash, anRSdesc.Flags);

      return hash;
    }
  //---------------------------------------------------------------------------//
    CommandListType locGetCommandListType(D3D12_COMMAND_LIST_TYPE aType)
    {
      switch(aType)
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

  CommandAllocatorPoolDX12* RenderCoreDX12::ourCommandAllocatorPool = nullptr;
  DescriptorHeapPoolDX12* RenderCoreDX12::ourDescriptorHeapPool = nullptr;
  ComPtr<ID3D12Device> RenderCoreDX12::ourDevice;
  ComPtr<ID3D12CommandQueue> RenderCoreDX12::ourCommandQueues[(uint) CommandListType::NUM];
  FenceDX12 RenderCoreDX12::ourCmdListDoneFences[(uint) CommandListType::NUM];
//---------------------------------------------------------------------------//
  void RenderCoreDX12::InitPlatform()
  {
    using namespace Microsoft::WRL;

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&ourDevice)));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CheckD3Dcall(ourDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ourCommandQueues[(uint) CommandListType::Graphics])));

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    CheckD3Dcall(ourDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&ourCommandQueues[(uint) CommandListType::Compute])));

    // Create synchronization objects.
    ourCmdListDoneFences[(uint) CommandListType::Graphics].Init(ourDevice.Get(), "RenderCoreDX12::GraphicsCommandListFinished");
    ourCmdListDoneFences[(uint)CommandListType::Compute].Init(ourDevice.Get(), "RenderCoreDX12::ComputeCommandListFinished");

    ourCommandAllocatorPool = new CommandAllocatorPoolDX12(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ourDescriptorHeapPool = new DescriptorHeapPoolDX12();
  }
//---------------------------------------------------------------------------//
  void RenderCoreDX12::ShutdownPlatform()
  {
  }
//---------------------------------------------------------------------------//
  Rendering::ShaderResourceInterface* RenderCoreDX12::GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS /* = nullptr */)
  {
    const uint& requestedHash = locComputeRSdescHash(anRSdesc);

    for (auto& rs : locShaderResourceInterfacePool)
      if (rs->myHash == requestedHash)
        return rs.get();

    ShaderResourceInterface* rs = new ShaderResourceInterface;
    locShaderResourceInterfacePool.push_back(std::unique_ptr<ShaderResourceInterface>(rs));

    rs->myHash = requestedHash;
    rs->myInterfaceDesc = locGetInterfaceDescFromRSdesc(anRSdesc);

    if (anRS == nullptr)
    {
      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      ComPtr<ID3D12RootSignature> rootSignature;
      HRESULT success = D3D12SerializeRootSignature(&anRSdesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
      ASSERT(success == S_OK, "Failed serializing RootSignature");

      ASSERT(ourDevice);

      success = ourDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
      ASSERT(success == S_OK, "Failed creating RootSignature");

      rs->myRootSignature = rootSignature;
    }
    else
    {
      rs->myRootSignature = anRS;
    }

    return rs;
  }
//---------------------------------------------------------------------------//
  void RenderCoreDX12::WaitForFence(CommandListType aType, uint64 aFenceVal)
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
  bool RenderCoreDX12::IsFenceDone(CommandListType aType, uint64 aFenceVal)
  {
    return ourCmdListDoneFences[(uint)aType].IsDone(aFenceVal);
  }
//---------------------------------------------------------------------------//
  uint64 RenderCoreDX12::ExecuteCommandList(ID3D12CommandList* aCommandList)
  {
    CommandListType type = locGetCommandListType(aCommandList->GetType());

    ID3D12CommandQueue* cmdQueue = ourCommandQueues[(uint)type].Get();
    FenceDX12& cmdListDoneFence = ourCmdListDoneFences[(uint)type];

    cmdListDoneFence.wait();
    cmdQueue->ExecuteCommandLists(1, &aCommandList);
    return cmdListDoneFence.signal(cmdQueue);
  }

//---------------------------------------------------------------------------//
  DataFormatInfo RenderCoreDX12::GetFormatInfo(DXGI_FORMAT aFormat)
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
  DataFormat RenderCoreDX12::ResolveFormat(DataFormat aFormat)
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
//---------------------------------------------------------------------------//
  DXGI_FORMAT RenderCoreDX12::GetFormat(DataFormat aFormat)
  {
      DataFormat supportedFormat = ResolveFormat(aFormat);
      
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

#endif