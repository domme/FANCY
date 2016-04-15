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
  RendererDX12::RendererDX12(void* aNativeWindowHandle)
    : myCommandAllocatorPool(nullptr)
    , myDefaultContext(nullptr)
	{
    CreateDeviceAndSwapChain(aNativeWindowHandle);
    
    // TODO: Get rid of this ugly and dangerous madness:
    Renderer& thisRenderer = *static_cast<Renderer*>(this);

    myCommandAllocatorPool = new CommandAllocatorPoolDX12(thisRenderer, D3D12_COMMAND_LIST_TYPE_DIRECT);
    myDefaultContext = new RenderContext(thisRenderer);
    myDescriptorHeapPool = new DescriptorHeapPoolDX12(*this);
	}
//---------------------------------------------------------------------------//
	RendererDX12::~RendererDX12()
	{
    SAFE_DELETE(myDefaultContext);
    SAFE_DELETE(myCommandAllocatorPool);
    SAFE_DELETE(myDescriptorHeapPool);
	}
//---------------------------------------------------------------------------//
  DescriptorDX12 RendererDX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType)
  {
    return myDescriptorHeapPool->GetStaticHeap(aHeapType)->AllocateDescriptor();
  }
//---------------------------------------------------------------------------// 
  uint64 RendererDX12::ExecuteCommandList(ID3D12CommandList* aCommandList)
  {
    myFence.wait();

    myCommandQueue->ExecuteCommandLists(1, &aCommandList);
    return myFence.signal(myCommandQueue.Get());
  }
//---------------------------------------------------------------------------//
  void RendererDX12::CreateDeviceAndSwapChain(void* aNativeWindowHandle)
  {
    using namespace Microsoft::WRL;

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    CheckD3Dcall(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice)));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CheckD3Dcall(myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue)));

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
    CheckD3Dcall(dxgiFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain));
    CheckD3Dcall(swapChain.As(&mySwapChain));
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();

    // Create synchronization objects.
    myFence.Init(myDevice.Get(), "RendererDX12::FrameDone");
  }
//---------------------------------------------------------------------------//
  void RendererDX12::CreateBackbufferResources()
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
      myDevice->CreateRenderTargetView(backbufferResource.myResource.Get(), nullptr, backbufferResource.myRtvDescriptor.myCpuHandle);
    }
  }
//---------------------------------------------------------------------------//
  void RendererDX12::postInit()
	{
    CreateBackbufferResources();
	}
//---------------------------------------------------------------------------//
  void RendererDX12::beginFrame()
	{
    myFence.wait();
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();

    Texture& currBackbuffer = myBackbuffers[myCurrBackbufferIndex];
    myDefaultContext->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    myDefaultContext->ClearRenderTarget(&currBackbuffer, clearColor);

    const float clearDepth = 1.0f;
    uint8 clearStencil = 0u;
    myDefaultContext->ClearDepthStencilTarget(&myDefaultDepthStencil, clearDepth, clearStencil);

    myDefaultContext->ExecuteAndReset(false);
	}
//---------------------------------------------------------------------------//
	void RendererDX12::endFrame()
	{
    TextureDX12& currBackbuffer = myBackbuffers[myCurrBackbufferIndex];

    //myDefaultContext->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

    myDefaultContext->TransitionResource(&currBackbuffer, D3D12_RESOURCE_STATE_PRESENT, true);
    myDefaultContext->ExecuteAndReset(false);

    mySwapChain->Present(1, 0);
	}
//---------------------------------------------------------------------------//
  void RendererDX12::WaitForFence(uint64 aFenceVal)
  {
    if (myFence.IsDone(aFenceVal))
      return;

    if (myFence.GetCurrWaitingFenceVal() >= aFenceVal)
    {
      myFence.wait();
    }
    else
    {
      myFence.signal(myCommandQueue.Get(), aFenceVal);
      myFence.wait();
    }
  }
//---------------------------------------------------------------------------//
  void RenderCoreDX12::InitPlatform()
  {
    ShaderResourceInterfacePoolDX12::Init();
  }
//---------------------------------------------------------------------------//
  void RenderCoreDX12::ShutdownPlatform()
  {
    ShaderResourceInterfacePoolDX12::Destroy();
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
    std::vector<std::unique_ptr<ShaderResourceInterface>> locShaderResourceInterfacePool;
  }  // namespace
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  Rendering::ShaderResourceInterface* RenderCoreDX12::GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS = nullptr)
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

      ID3D12Device* device = Fancy::GetRenderer()->GetDevice();
      ASSERT(device);

      success = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
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
} } }  // end of namespace Fancy::Rendering::DX12

#endif