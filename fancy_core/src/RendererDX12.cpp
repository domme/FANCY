#include "RendererDX12.h"

#if defined (RENDERER_DX12)
#include "MathUtil.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering { namespace DX12 { 

  PipelineState::PipelineState()
    : myDepthStencilState(ObjectName::blank)
    , myBlendState(ObjectName::blank)
  {
      
  }

  uint PipelineState::getHash()
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, static_cast<uint>(myFillMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myCullMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myWindingOrder));
    MathUtil::hash_combine(hash, myDepthStencilState.getHash());
    MathUtil::hash_combine(hash, myBlendState.getHash());

    for (uint i = 0u; i < static_cast<uint>(ShaderStage::NUM); ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myShaderStages[i]));

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }

  void PipelineState::fillNativePSOdesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& aDesc)
  {
    memset(&aDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    aDesc.pRootSignature = nullptr;  // TODO

    D3D12_SHADER_BYTECODE* shaderDescs[]{ &aDesc.VS, &aDesc.PS, &aDesc.DS, &aDesc.HS, &aDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
    {
      if (nullptr == myShaderStages[i])
        continue;

      (*shaderDescs[i]) = myShaderStages[i]->getNativeByteCode();
    }
    
    D3D12_BLEND_DESC blendDesc;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = myBlendState.getAlphaToCoverageEnabled();
    blendDesc.IndependentBlendEnable = myBlendState.getBlendStatePerRT();
    uint rtCount = blendDesc.IndependentBlendEnable ? kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));

	  rtBlendDesc.BlendEnable = myBlendState.myBlendEnabled[rt];
	  //rtBlendDesc.BlendOp 
	  


    }

  }

RendererDX12::RendererDX12()
	{

	}

	RendererDX12::~RendererDX12()
	{
	
	}

  void RendererDX12::init(void* aNativeWindowHandle)
  {
    using namespace Microsoft::WRL;

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
      debugInterface->EnableDebugLayer();

    HRESULT success = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    success = myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue));

    HWND windowHandle = *static_cast<HWND*>(aNativeWindowHandle);

    ComPtr<IDXGIFactory4> dxgiFactory;
    success = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

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
    success = dxgiFactory->CreateSwapChain(myCommandQueue.Get(), &swapChainDesc, &swapChain);
    success = swapChain.As(&mySwapChain);
    myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
      // Describe and create a render target view (RTV) descriptor heap.
      D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
      rtvHeapDesc.NumDescriptors = kBackbufferCount;
      rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      success = myDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&myRtvHeap));

      myRtvDescriptorSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
      uint rtvHandleOffset = 0;

      // Create a RTV for each backbuffer.
      
      for (UINT n = 0; n < kBackbufferCount; n++)
      {
        mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackbuffers[n]));
        myDevice->CreateRenderTargetView(myBackbuffers[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += myRtvDescriptorSize;
      }
    }

    myDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&myCommandAllocator));
  }

  void RendererDX12::postInit()
	{
    myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, myCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&myCommandList));
    myCommandList->Close();

    // Create synchronization objects.
    myFrameDone.init(myDevice.Get(), "RendererDX12::FrameDone");
	}
	
  PipelineState& RendererDX12::getState()
  {
    // TODO: Retrieve the state-object of the current thread here...
    return myState;
  }
  
  void RendererDX12::beginFrame()
	{
    myFrameDone.wait();

    myCommandAllocator->Reset();
    myCommandList->Reset(myCommandAllocator.Get(), nullptr);
    myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
	}
	
	void RendererDX12::endFrame()
	{
    // Move this part to rendering
    ////////////////////////////////////////////////////////////////////////////////
    D3D12_RESOURCE_BARRIER bbBarrier;
    bbBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    bbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    bbBarrier.Transition.pResource = myBackbuffers[myFrameIndex].Get();
    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    myCommandList->ResourceBarrier(1, &bbBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
    backbufferHandle.ptr += myFrameIndex * myRtvDescriptorSize;

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    myCommandList->ClearRenderTargetView(backbufferHandle, clearColor, 0, nullptr);

    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    myCommandList->ResourceBarrier(1, &bbBarrier);
    
    myCommandList->Close();
    ////////////////////////////////////////////////////////////////////////////////

    ID3D12CommandList* commandLists[] = { myCommandList.Get() };
    myCommandQueue->ExecuteCommandLists(1, commandLists);
    mySwapChain->Present(1, 0);

    myFrameDone.signal(myCommandQueue.Get());
	}
	
	void RendererDX12::setViewport(const glm::uvec4& uViewportParams)
	{
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
	}
	
	void RendererDX12::setBlendState(const BlendState& clBlendState)
	{
    PipelineState& state = getState();

    if (state.myBlendState.m_uHash == clBlendState.m_uHash)
      return;

    state.myBlendState = clBlendState;
    state.myIsDirty = true;
	}
	
	void RendererDX12::setDepthStencilState(const DepthStencilState& clDepthStencilState)
	{
    PipelineState& state = getState();

    if (state.myDepthStencilState.m_uHash = clDepthStencilState.m_uHash)
      return;

    state.myDepthStencilState = clDepthStencilState;
    state.myIsDirty = true;
	}
	
	void RendererDX12::setFillMode(const FillMode eFillMode)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
	}
	
	void RendererDX12::setCullMode(const CullMode eCullMode)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
	}
	
	void RendererDX12::setWindingOrder(const WindingOrder eWindingOrder)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
	}
	
	void RendererDX12::setDepthStencilRenderTarget(Texture* pDStexture)
	{
    
	}
	
	void RendererDX12::setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
	{

	}
	
	void RendererDX12::removeAllRenderTargets()
	{
	}
	
	void RendererDX12::setReadTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex)
	{
	}
	
	void RendererDX12::setWriteTexture(const Texture* pTexture, const ShaderStage eShaderStage, const uint8 u8RegisterIndex)
	{
	}
	
	void RendererDX12::setReadBuffer(const GpuBuffer* pBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex)
	{
	}
	
	void RendererDX12::setConstantBuffer(const GpuBuffer* pConstantBuffer, const ShaderStage eShaderStage, const uint8 u8RegisterIndex)
	{
	}
	
	void RendererDX12::setTextureSampler(const TextureSampler* pSampler, const ShaderStage eShaderStage, const uint8 u8RegisterIndex)
	{
	}
	
	void RendererDX12::setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage)
	{
    PipelineState& state = getState();

    if (state.myShaderStages[(uint)eShaderStage] == pProgram)
      return;

    state.myShaderStages[(uint)eShaderStage] = pProgram;
    state.myIsDirty = true;
	}
	
	void RendererDX12::renderGeometry(const Geometry::GeometryData* pGeometry)
	{

	}

#pragma region Pipeline Apply

  void RendererDX12::applyViewport()
  {

  }

  void RendererDX12::applyPipelineState()
	{
    PipelineState& requestedState = getState();

    if (!requestedState.myIsDirty)
      return;

    uint requestedHash = requestedState.getHash();

    ID3D12PipelineState* pso = nullptr;

    auto cachedPSOIter = myPSOcache.find(requestedHash);
    if (cachedPSOIter != myPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      
    }

    myCommandList->SetPipelineState(cachedPSOIter->second);






	}


#pragma endregion 
	
} } }  // end of namespace Fancy::Rendering::DX12

#endif