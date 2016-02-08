#include "RendererDX12.h"
#include "AdapterDX12.h"

#if defined (RENDERER_DX12)
#include "MathUtil.h"
#include "GpuProgram.h"
#include "RootSignatureDX12.h"
#include "GpuProgramCompiler.h"

namespace Fancy { namespace Rendering { namespace DX12 { 
//---------------------------------------------------------------------------//
  PipelineState::PipelineState()
    : myFillMode(FillMode::SOLID)
    , myCullMode(CullMode::BACK)
    , myWindingOrder(WindingOrder::CCW)
    , myNumRenderTargets(0u)
    , myDSVformat(DataFormat::DS_24_8)
    , myIsDirty(true)
  {
  }
//---------------------------------------------------------------------------//
  uint PipelineState::getHash()
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, static_cast<uint>(myFillMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myCullMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myWindingOrder));
    MathUtil::hash_combine(hash, myDepthStencilState.GetHash());
    MathUtil::hash_combine(hash, myBlendState.GetHash());

    for (uint i = 0u; i < static_cast<uint>(ShaderStage::NUM); ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myShaderStages[i]));

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));
  
    MathUtil::hash_combine(hash, myInputLayout.myHash);

    return hash;
  }
//---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineState::toNativePSOdesc()
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.DS, &psoDesc.HS, &psoDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
    {
      if (nullptr == myShaderStages[i])
        continue;

      (*shaderDescs[i]) = myShaderStages[i]->getNativeByteCode();
    }

    // ROOT SIGNATURE
    psoDesc.pRootSignature = nullptr;  // Don't override the RS - just use the embedded one in the shader stages
    
    // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = myBlendState.getAlphaToCoverageEnabled();
    blendDesc.IndependentBlendEnable = myBlendState.getBlendStatePerRT();
    uint rtCount = blendDesc.IndependentBlendEnable ? Constants::kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));

	    rtBlendDesc.BlendEnable = myBlendState.myBlendEnabled[rt];
      rtBlendDesc.BlendOp = Adapter::toNativeType(myBlendState.myBlendOp[rt]);
      rtBlendDesc.BlendOpAlpha = Adapter::toNativeType(myBlendState.myBlendOpAlpha[rt]);
      rtBlendDesc.DestBlend = Adapter::toNativeType(myBlendState.myDestBlend[rt]);
      rtBlendDesc.DestBlendAlpha = Adapter::toNativeType(myBlendState.myDestBlendAlpha[rt]);

      // FEATURE: Add support for LogicOps?
      rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
      rtBlendDesc.LogicOpEnable = false;

      if (myBlendState.myRTwriteMask[rt] & 0xFFFFFF > 0u)
      {
        rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      }
      else
      {
        const bool red = (myBlendState.myRTwriteMask[rt] & 0xFF000000) > 0u;
        const bool green = (myBlendState.myRTwriteMask[rt] & 0x00FF0000) > 0u;
        const bool blue = (myBlendState.myRTwriteMask[rt] & 0x0000FF00) > 0u;
        const bool alpha = (myBlendState.myRTwriteMask[rt] & 0x000000FF) > 0u;
        rtBlendDesc.RenderTargetWriteMask |= red ? D3D12_COLOR_WRITE_ENABLE_RED : 0u;
        rtBlendDesc.RenderTargetWriteMask |= green ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0u;
        rtBlendDesc.RenderTargetWriteMask |= blue ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0u;
        rtBlendDesc.RenderTargetWriteMask |= alpha ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0u;
      }
    }

    // STREAM OUTPUT
    // FEATURE: Add support for StreamOutput
    D3D12_STREAM_OUTPUT_DESC& streamOutDesc = psoDesc.StreamOutput;
    memset(&streamOutDesc, 0u, sizeof(D3D12_STREAM_OUTPUT_DESC));

    // SAMPLE MASK / DESC
    psoDesc.SampleMask = ~0u;
    psoDesc.SampleDesc.Count = 1u;

    // RASTERIZER STATE
    D3D12_RASTERIZER_DESC& rasterizerDesc = psoDesc.RasterizerState;
    memset(&rasterizerDesc, 0u, sizeof(D3D12_RASTERIZER_DESC));
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    rasterizerDesc.FillMode = Adapter::toNativeType(myFillMode);
    rasterizerDesc.CullMode = Adapter::toNativeType(myCullMode);
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.FrontCounterClockwise = myWindingOrder == WindingOrder::CCW;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = false;

    // DEPTH STENCIL STATE
    D3D12_DEPTH_STENCIL_DESC& dsState = psoDesc.DepthStencilState;
    dsState.DepthEnable = myDepthStencilState.myDepthTestEnabled;
    dsState.DepthWriteMask = myDepthStencilState.myDepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    dsState.DepthFunc = Adapter::toNativeType(myDepthStencilState.myDepthCompFunc);
    dsState.StencilEnable = myDepthStencilState.myStencilEnabled;
    dsState.StencilReadMask = static_cast<uint8>(myDepthStencilState.myStencilReadMask);
    dsState.StencilWriteMask = static_cast<uint8>(myDepthStencilState.myStencilWriteMask[0u]);
    // FrontFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.FrontFace;
      uint faceIdx = static_cast<uint>(FaceType::FRONT);
      faceDesc.StencilFunc = Adapter::toNativeType(myDepthStencilState.myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(myDepthStencilState.myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(myDepthStencilState.myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(myDepthStencilState.myStencilPassOp[faceIdx]);
    }
    // BackFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.BackFace;
      uint faceIdx = static_cast<uint>(FaceType::BACK);
      faceDesc.StencilFunc = Adapter::toNativeType(myDepthStencilState.myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(myDepthStencilState.myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(myDepthStencilState.myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(myDepthStencilState.myStencilPassOp[faceIdx]);
    }

    // INPUT LAYOUT
    ASSERT_M(!myInputLayout.myElements.empty(), "Invalid input layout");
    D3D12_INPUT_LAYOUT_DESC& inputLayout = psoDesc.InputLayout;
    inputLayout.NumElements = myInputLayout.myElements.size();
    inputLayout.pInputElementDescs = &myInputLayout.myElements[0u];

    // IB STRIP CUT VALUE
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // TOPOLOGY TYPE
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // NUM RENDER TARGETS
    psoDesc.NumRenderTargets = myNumRenderTargets;

    // RTV-FORMATS
    for (uint i = 0u; i < myNumRenderTargets; ++i)
    {
      psoDesc.RTVFormats[i] = Adapter::toNativeType(myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = Adapter::toNativeType(myDSVformat);

    // NODE MASK
    psoDesc.NodeMask = 0u;

    return psoDesc;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RendererDX12::RendererDX12()
	{

	}
//---------------------------------------------------------------------------//
	RendererDX12::~RendererDX12()
	{
	
	}
//---------------------------------------------------------------------------//
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
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();

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
//---------------------------------------------------------------------------//
  void RendererDX12::postInit()
	{
    for (uint i = 0u; i < Constants::kNumRenderThreads; ++i)
    {
      myDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, myCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&myCommandList[i]));
      myCommandList[i]->Close();
    }

    // Create synchronization objects.
    myFrameDone.init(myDevice.Get(), "RendererDX12::FrameDone");

    // DEBUG: Compile a shader
    GpuProgramPermutation permutation;

    GpuProgramDesc vertexProgramDesc;
    vertexProgramDesc.myPermutation = permutation;
    vertexProgramDesc.myShaderPath = Rendering::GpuProgramCompiler::GetPlatformShaderFileDirectory() +
      String("MaterialForward") + Rendering::GpuProgramCompiler::GetPlatformShaderFileExtension();
    vertexProgramDesc.myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
    GpuProgram* pVertexProgram = GpuProgramCompiler::createOrRetrieve(vertexProgramDesc);
	}
//---------------------------------------------------------------------------//
  PipelineState& RendererDX12::getState()
  {
    return myState[getCurrentRenderThreadIdx()];
  }
//---------------------------------------------------------------------------//
  ComPtr<ID3D12GraphicsCommandList>& RendererDX12::getGraphicsCmdList()
  {
    return myCommandList[getCurrentRenderThreadIdx()];
  }
//---------------------------------------------------------------------------//
  void RendererDX12::beginFrame()
	{
    ComPtr<ID3D12GraphicsCommandList>&  cmdList = getGraphicsCmdList();

    myFrameDone.wait();

    myCommandAllocator->Reset();
    cmdList->Reset(myCommandAllocator.Get(), nullptr);
    myCurrBackbufferIndex = mySwapChain->GetCurrentBackBufferIndex();
	}
//---------------------------------------------------------------------------//
	void RendererDX12::endFrame()
	{
    ComPtr<ID3D12GraphicsCommandList>&  cmdList = getGraphicsCmdList();

    // TODO: Adapt this to multithreaded rendering: Wait until all cmd-lists are completed, patch them with transitional resource-barriers and execute them in-order

    // Move this part to rendering
    ////////////////////////////////////////////////////////////////////////////////
    D3D12_RESOURCE_BARRIER bbBarrier;
    bbBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    bbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    bbBarrier.Transition.pResource = myBackbuffers[myCurrBackbufferIndex].Get();
    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &bbBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle = myRtvHeap->GetCPUDescriptorHandleForHeapStart();
    backbufferHandle.ptr += myCurrBackbufferIndex * myRtvDescriptorSize;

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearRenderTargetView(backbufferHandle, clearColor, 0, nullptr);

    bbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &bbBarrier);
    
    cmdList->Close();
    ////////////////////////////////////////////////////////////////////////////////

    ID3D12CommandList* commandLists[] = { cmdList.Get() };
    myCommandQueue->ExecuteCommandLists(1, commandLists);
    mySwapChain->Present(1, 0);

    myFrameDone.signal(myCommandQueue.Get());
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setViewport(const glm::uvec4& uViewportParams)
	{
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setBlendState(const BlendState& clBlendState)
	{
    PipelineState& state = getState();
    uint requestedHash = clBlendState.GetHash();

    if (state.myBlendState.GetHash() == requestedHash)
      return;

    state.myBlendState = clBlendState;
    state.myIsDirty = true;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setDepthStencilState(const DepthStencilState& aDepthStencilState)
	{
    PipelineState& state = getState();
    uint requestedHash = aDepthStencilState.GetHash();

    if (state.myDepthStencilState.GetHash() == requestedHash)
      return;

    state.myDepthStencilState = aDepthStencilState;
    state.myIsDirty = true;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setFillMode(const FillMode eFillMode)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setCullMode(const CullMode eCullMode)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setWindingOrder(const WindingOrder eWindingOrder)
	{
    PipelineState& state = getState();
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setDepthStencilRenderTarget(Texture* pDStexture)
	{
    
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
	{
    
	}
//---------------------------------------------------------------------------//
	void RendererDX12::removeAllRenderTargets()
	{
	}
//---------------------------------------------------------------------------//
  void RendererDX12::SetGraphicsRootSignature(ID3D12RootSignature* aRootSignature)
  {
    ComPtr<ID3D12GraphicsCommandList>& cmdList = getGraphicsCmdList();
    cmdList->SetGraphicsRootSignature(aRootSignature);
  }
//---------------------------------------------------------------------------//
	void RendererDX12::setReadTexture(const Texture* pTexture, const uint8 u8RegisterIndex)
	{
    ComPtr<ID3D12GraphicsCommandList>& cmdList = getGraphicsCmdList();
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setWriteTexture(const Texture* pTexture, const uint8 u8RegisterIndex)
	{
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setReadBuffer(const GpuBuffer* pBuffer, const uint8 u8RegisterIndex)
	{
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setConstantBuffer(const GpuBuffer* pConstantBuffer, const uint8 u8RegisterIndex)
	{
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setTextureSampler(const TextureSampler* pSampler, const uint8 u8RegisterIndex)
	{
	}
//---------------------------------------------------------------------------//
	void RendererDX12::setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage)
	{
    PipelineState& state = getState();

    if (state.myShaderStages[(uint)eShaderStage] == pProgram)
      return;

    state.myShaderStages[(uint)eShaderStage] = pProgram;
    state.myIsDirty = true;
	}
//---------------------------------------------------------------------------//
	void RendererDX12::renderGeometry(const Geometry::GeometryData* pGeometry)
	{

	}

//---------------------------------------------------------------------------//
  void RendererDX12::CopySubresources(ID3D12Resource* aDestResource, ID3D12Resource* aSrcResource, uint aFirstSubresource, uint aSubResourceCount)
  {
    ComPtr<ID3D12GraphicsCommandList>& cmdList = getGraphicsCmdList();

    //CD3DX12_RESOURCE_BARRIER copyPrepareTransitionBarriers[] =
    //{
    //  CD3DX12_RESOURCE_BARRIER::Transition(aDestResource, D3D12_Resource_state_)
    //}

    // cmdList->ResourceBarrier()


  }
//---------------------------------------------------------------------------//
  void RendererDX12::InitBufferData(GpuResourceDX12* aBuffer, void* aDataPtr)
  {
    
  }
//---------------------------------------------------------------------------//
#pragma region Pipeline Apply
//---------------------------------------------------------------------------//
  void RendererDX12::applyViewport()
  {

  }
//---------------------------------------------------------------------------//
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
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = requestedState.toNativePSOdesc();
      HRESULT result = myDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT_M(result == S_OK, "Error creating graphics PSO");
      
      myPSOcache[requestedHash] = pso;
    }
    requestedState.myIsDirty = false;

    ComPtr<ID3D12GraphicsCommandList>& cmdList = getGraphicsCmdList();
    cmdList->SetPipelineState(pso);
	}
//---------------------------------------------------------------------------//
#pragma endregion 
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void RenderingSubsystemDX12::InitPlatform()
  {
    RootSignaturePoolDX12::Init();
  }
//---------------------------------------------------------------------------//
  void RenderingSubsystemDX12::ShutdownPlatform()
  {
    RootSignaturePoolDX12::Destroy();
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::DX12

#endif