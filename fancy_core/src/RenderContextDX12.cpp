#include "FancyCorePrerequisites.h"

#include "RenderContextDX12.h"
#include "MathUtil.h"
#include "AdapterDX12.h"
#include "GpuProgram.h"
#include "Renderer.h"
#include "DescriptorHeapPoolDX12.h"
#include <unordered_set>
#include "GpuBuffer.h"
#include "Fancy.h"
#include "Texture.h"
#include "TextureSampler.h"
#include "GeometryData.h"
#include "GpuProgramPipeline.h"
#include "RootSignatureDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  PipelineState::PipelineState()
    : myFillMode(FillMode::SOLID)
    , myCullMode(CullMode::BACK)
    , myWindingOrder(WindingOrder::CCW)
    , myNumRenderTargets(0u)
    , myDSVformat(DataFormat::UNKNOWN)
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

    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myGpuProgramPipeline));

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }
  //---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineState::GetNativePSOdesc()
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.DS, &psoDesc.HS, &psoDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    if (myGpuProgramPipeline != nullptr)
    {
      for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
      {
        if (nullptr == myGpuProgramPipeline->myGpuPrograms[i])
          continue;

        (*shaderDescs[i]) = myGpuProgramPipeline->myGpuPrograms[i]->getNativeByteCode();
      }
    }
    
    // ROOT SIGNATURE
    psoDesc.pRootSignature = myGpuProgramPipeline->myRootSignature->myRootSignature.Get();

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

    if (myGpuProgramPipeline != nullptr &&
        myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX] != nullptr)
    {
      const GpuProgram* vertexShader =
        myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX];

      D3D12_INPUT_LAYOUT_DESC& inputLayout = psoDesc.InputLayout;
      inputLayout.NumElements = vertexShader->GetNumNativeInputElements();
      inputLayout.pInputElementDescs = vertexShader->GetNativeInputElements();
    }

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
  std::unordered_map<uint, ID3D12PipelineState*> RenderContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  ResourceState::ResourceState() 
    : myDirtyFlags(0u)
    , myReadTexturesRebindCount(0u)
    , myConstantBufferRebindCount(0u)
    , myTextureSamplerRebindCount(0u)
  {
    memset(myReadTextures, 0, sizeof(myReadTextures));
    memset(myConstantBuffers, 0, sizeof(myConstantBuffers));
    memset(myTextureSamplers, 0, sizeof(myTextureSamplers));
  }
//---------------------------------------------------------------------------// 
  RenderContextDX12::RenderContextDX12()
    : myRenderer(*Fancy::GetRenderer())
    , myCommandAllocatorPool(*myRenderer.GetCommandAllocatorPool())
    , myViewportParams(0, 0, 1, 1)
    , myViewportDirty(true)
    , myRootSignature(nullptr)
    , myRootSignatureDirty(false)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myCpuVisibleAllocator(myRenderer, GpuDynamicAllocatorType::CpuWritable)
    , myGpuOnlyAllocator(myRenderer, GpuDynamicAllocatorType::GpuOnly)
    , myIsInRecordState(true)
    , myRenderTargetsDirty(true)
    , myDepthStencilTarget(nullptr)
  {
    ResetInternalStates();

    ID3D12Device* device = myRenderer.GetDevice();
    
    myCommandAllocator = myCommandAllocatorPool.GetNewAllocator();

    CheckD3Dcall(
      device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
        myCommandAllocator, nullptr, IID_PPV_ARGS(&myCommandList))
      );    
  }
//---------------------------------------------------------------------------//
  RenderContextDX12::RenderContextDX12(Renderer& aRenderer)
    : myRenderer(aRenderer)
    , myCommandAllocatorPool(*myRenderer.GetCommandAllocatorPool())
    , myViewportParams(0, 0, 1, 1)
    , myViewportDirty(true)
    , myRootSignature(nullptr)
    , myRootSignatureDirty(false)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myCpuVisibleAllocator(myRenderer, GpuDynamicAllocatorType::CpuWritable)
    , myGpuOnlyAllocator(myRenderer, GpuDynamicAllocatorType::GpuOnly)
    , myIsInRecordState(true)
    , myRenderTargetsDirty(true)
    , myDepthStencilTarget(nullptr)
  {
    ResetInternalStates();

    ID3D12Device* device = myRenderer.GetDevice();

    myCommandAllocator = myCommandAllocatorPool.GetNewAllocator();

    CheckD3Dcall(
      device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        myCommandAllocator, nullptr, IID_PPV_ARGS(&myCommandList))
      );
  }
//---------------------------------------------------------------------------//
  RenderContextDX12::~RenderContextDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::Destroy()
  {
    if (myCommandList != nullptr)
      myCommandList->Release(); 
    myCommandList = nullptr;

    if (myCommandAllocator != nullptr)
      ReleaseAllocator(0u);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ResetInternalStates()
  {
    myRootSignature = nullptr;
    myRootSignatureDirty = true;
    myPipelineState = PipelineState();
    myResourceState = ResourceState();
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myDynamicShaderVisibleHeaps, 0u, sizeof(myDynamicShaderVisibleHeaps));
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::Reset()
  {
    if (myIsInRecordState)
      return;

    ASSERT_M(nullptr == myCommandAllocator, "myIsInRecordState-flag out of sync");

    myCommandAllocator = myCommandAllocatorPool.GetNewAllocator();
    ASSERT(myCommandAllocator != nullptr);

    CheckD3Dcall(myCommandList->Reset(myCommandAllocator, nullptr));

    myIsInRecordState = true;

    ResetInternalStates();
  }
//---------------------------------------------------------------------------//
  uint64 RenderContextDX12::ExecuteAndReset(bool aWaitForCompletion)
  {
    KickoffResourceBarriers();

    ASSERT(myCommandAllocator != nullptr && myCommandList != nullptr);
    CheckD3Dcall(myCommandList->Close());
    myIsInRecordState = false;

    uint64 fenceVal = myRenderer.ExecuteCommandList(myCommandList);

    myCpuVisibleAllocator.CleanupAfterCmdListExecute(fenceVal);
    myGpuOnlyAllocator.CleanupAfterCmdListExecute(fenceVal);
    ReleaseDynamicHeaps(fenceVal);
    ReleaseAllocator(fenceVal);

    if (aWaitForCompletion)
      myRenderer.WaitForFence(fenceVal);

    Reset();

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap)
  {
    if (myDynamicShaderVisibleHeaps[aHeapType] == aDescriptorHeap)
      return;

    // Has this heap already been used on this commandList? Then we need to "remember" it until ExecuteAndReset()
    if (myDynamicShaderVisibleHeaps[aHeapType] != nullptr)
      myRetiredDescriptorHeaps.push_back(myDynamicShaderVisibleHeaps[aHeapType]);

    myDynamicShaderVisibleHeaps[aHeapType] = aDescriptorHeap;
    ApplyDescriptorHeaps();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ClearRenderTarget(Texture* aTexture, const float* aColor)
  {
    ASSERT(aTexture->getParameters().myIsRenderTarget);
    myCommandList->ClearRenderTargetView(aTexture->GetRtv().myCpuHandle, aColor, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, 
    uint8 aStencilClear, uint32 someClearFlags /* = (uint32)DepthStencilClearFlags::CLEAR_ALL */)
  {
    ASSERT(aTexture->getParameters().bIsDepthStencil);

    D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS) 0;
    if (someClearFlags & (uint32)DepthStencilClearFlags::CLEAR_DEPTH)
      clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if (someClearFlags & (uint32)DepthStencilClearFlags::CLEAR_STENCIL)
      clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    myCommandList->ClearDepthStencilView(aTexture->GetDsv().myCpuHandle, clearFlags, aDepthClear, aStencilClear, 0, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::TransitionResource(GpuResourceDX12* aResource, D3D12_RESOURCE_STATES aDestState, bool aExecuteNow /* = false */)
  {
    if (aResource->GetUsageState() == aDestState)
      return;

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = aResource->GetResource();
    barrier.Transition.StateBefore = aResource->GetUsageState();
    barrier.Transition.StateAfter = aDestState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    aResource->myUsageState = aDestState;

    myWaitingResourceBarriers.push_back(barrier);

    if (aExecuteNow || myWaitingResourceBarriers.IsFull())
    {
      KickoffResourceBarriers();
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyDescriptorHeaps()
  {
    ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    memset(heapsToBind, 0, sizeof(heapsToBind));
    uint32 numHeapsToBind = 0u;
    
    for(DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
      if (heap != nullptr)
        heapsToBind[numHeapsToBind++] = heap->GetHeap();
    
    if (numHeapsToBind > 0u)
      myCommandList->SetDescriptorHeaps(numHeapsToBind, heapsToBind);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::KickoffResourceBarriers()
  {
    if (myWaitingResourceBarriers.empty())
      return;

    myCommandList->ResourceBarrier(myWaitingResourceBarriers.size(), &myWaitingResourceBarriers[0]);
    myWaitingResourceBarriers.clear();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ReleaseAllocator(uint64 aFenceVal)
  {
    myCommandAllocatorPool.ReleaseAllocator(myCommandAllocator, aFenceVal);
    myCommandAllocator = nullptr;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ReleaseDynamicHeaps(uint64 aFenceVal)
  {
    DescriptorHeapPoolDX12* heapPool = myRenderer.GetDescriptorHeapPool();

    for (DescriptorHeapDX12* heap : myDynamicShaderVisibleHeaps)
    {
      if (heap != nullptr)
        heapPool->ReleaseDynamicHeap(aFenceVal, heap);
    }

    for (DescriptorHeapDX12* heap : myRetiredDescriptorHeaps)
    {
      heapPool->ReleaseDynamicHeap(aFenceVal, heap);
    }

    myRetiredDescriptorHeaps.clear();
    memset(myDynamicShaderVisibleHeaps, 0, sizeof(myDynamicShaderVisibleHeaps));
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::setViewport(const glm::uvec4& uViewportParams)
  {
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setBlendState(const BlendState& clBlendState)
  {
    PipelineState& state = myPipelineState;
    uint requestedHash = clBlendState.GetHash();

    if (state.myBlendState.GetHash() == requestedHash)
      return;

    state.myBlendState = clBlendState;
    state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setDepthStencilState(const DepthStencilState& aDepthStencilState)
  {
    PipelineState& state = myPipelineState;
    uint requestedHash = aDepthStencilState.GetHash();

    if (state.myDepthStencilState.GetHash() == requestedHash)
      return;

    state.myDepthStencilState = aDepthStencilState;
    state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setFillMode(const FillMode eFillMode)
  {
    PipelineState& state = myPipelineState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setCullMode(const CullMode eCullMode)
  {
    PipelineState& state = myPipelineState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setWindingOrder(const WindingOrder eWindingOrder)
  {
    PipelineState& state = myPipelineState;
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setDepthStencilRenderTarget(Texture* pDStexture)
  {
    if (myDepthStencilTarget == pDStexture)
      return;

    ASSERT(pDStexture->getParameters().bIsDepthStencil);

    myDepthStencilTarget = pDStexture;
    myRenderTargetsDirty = true;

    myPipelineState.myDSVformat = pDStexture->getParameters().eFormat;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
  {
    if (myRenderTargets[u8RenderTargetIndex] == pRTTexture)
      return;

    myRenderTargets[u8RenderTargetIndex] = pRTTexture;
    myRenderTargetsDirty = true;

    myPipelineState.myNumRenderTargets = glm::max(myPipelineState.myNumRenderTargets, (uint8) (u8RenderTargetIndex + 1));
    myPipelineState.myRTVformats[u8RenderTargetIndex] = pRTTexture->getParameters().eFormat;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::removeAllRenderTargets()
  {
    memset(myRenderTargets, 0, sizeof(myRenderTargets));
    myDepthStencilTarget = nullptr;
    myRenderTargetsDirty = true;

    myPipelineState.myNumRenderTargets = 0u;
    for (uint32 i = 0u; i < ARRAY_LENGTH(myPipelineState.myRTVformats); ++i)
      myPipelineState.myRTVformats[i] = DataFormat::NONE;

    myPipelineState.myDSVformat = DataFormat::NONE;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::setReadTexture(const Texture* pTexture, const uint8 u8RegisterIndex)
  {
    if (myResourceState.myReadTextures[u8RegisterIndex] == pTexture)
      return;

    myResourceState.myReadTextures[u8RegisterIndex] = pTexture;
    myResourceState.myDirtyFlags |= ResourceState::READ_TEXTURES_DIRTY;
    myResourceState.myReadTexturesRebindCount = 
      glm::max(myResourceState.myReadTexturesRebindCount, (uint32) u8RegisterIndex + 1u);
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setWriteTexture(const Texture* pTexture, const uint8 u8RegisterIndex)
  {
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setReadBuffer(const GpuBuffer* pBuffer, const uint8 u8RegisterIndex)
  {
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setConstantBuffer(const GpuBuffer* pConstantBuffer, const uint8 u8RegisterIndex)
  {
    if (myResourceState.myConstantBuffers[u8RegisterIndex] == pConstantBuffer)
      return;

    myResourceState.myConstantBuffers[u8RegisterIndex] = pConstantBuffer;
    myResourceState.myDirtyFlags |= ResourceState::CONSTANT_BUFFERS_DIRTY;
    myResourceState.myConstantBufferRebindCount =
      glm::max(myResourceState.myConstantBufferRebindCount, (uint32)u8RegisterIndex + 1u);
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setTextureSampler(const TextureSampler* pSampler, const uint8 u8RegisterIndex)
  {
    if (myResourceState.myTextureSamplers[u8RegisterIndex] == pSampler)
      return;

    myResourceState.myTextureSamplers[u8RegisterIndex] = pSampler;
    myResourceState.myDirtyFlags |= ResourceState::TEXTURE_SAMPLERS_DIRTY;
    myResourceState.myTextureSamplerRebindCount =
      glm::max(myResourceState.myTextureSamplerRebindCount, (uint32)u8RegisterIndex + 1u);
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::SetGpuProgramPipeline(const GpuProgramPipeline* aGpuProgramPipeline)
  {
    if (myPipelineState.myGpuProgramPipeline != aGpuProgramPipeline)
    {
      myPipelineState.myGpuProgramPipeline = aGpuProgramPipeline;
      myPipelineState.myIsDirty = true;
    }

    if (myRootSignature != aGpuProgramPipeline->GetRootSignatureNative())
    {
      myRootSignature = aGpuProgramPipeline->GetRootSignatureNative();
      myRootSignatureDirty = true;
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::renderGeometry(const Geometry::GeometryData* pGeometry)
  {
    ApplyViewport();
    ApplyRenderTargets();
    ApplyRootSignature();
    ApplyPipelineState();
    ApplyResourceState();

    myCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    myCommandList->IASetVertexBuffers(0, 1, &pGeometry->getVertexBuffer()->GetVertexBufferView());
    myCommandList->IASetIndexBuffer(&pGeometry->getIndexBuffer()->GetIndexBufferView());
    myCommandList->DrawIndexedInstanced(pGeometry->getNumIndices(), 1, 0, 0, 0);
  }
//---------------------------------------------------------------------------//
  static void locMemcpySubresourceRows(const D3D12_MEMCPY_DEST* aDest, const D3D12_SUBRESOURCE_DATA* aSrc, size_t aRowStrideBytes, uint32 aNumRows, uint32 aNumSlices)
  {
    for (uint32 iSlice = 0u; iSlice < aNumSlices; ++iSlice)
    {
      uint8* destSliceDataPtr = static_cast<uint8*>(aDest->pData) + aDest->SlicePitch * iSlice;
      const uint8* srcSliceDataPtr = static_cast<const uint8*>(aSrc->pData) + aSrc->SlicePitch * iSlice;
      for (uint32 iRow = 0u; iRow < aNumRows; ++iRow)
      {
        uint8* destDataPtr = destSliceDataPtr + aDest->RowPitch * iRow;
        const uint8* srcDataPtr = srcSliceDataPtr + aSrc->RowPitch * iRow;

        memcpy(destDataPtr, srcDataPtr, aRowStrideBytes);
      }
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::UpdateSubresources(ID3D12Resource* aDestResource, ID3D12Resource* aStagingResource, 
    uint32 aFirstSubresourceIndex, uint32 aNumSubresources, D3D12_SUBRESOURCE_DATA* someSubresourceDatas)
  {
    D3D12_RESOURCE_DESC srcDesc = aStagingResource->GetDesc();
    D3D12_RESOURCE_DESC destDesc = aDestResource->GetDesc();
    
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* destLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * aNumSubresources));
    uint64* destRowSizesByte = static_cast<uint64*>(alloca(sizeof(uint64) * aNumSubresources));
    uint32* destRowNums = static_cast<uint32*>(alloca(sizeof(uint32) * aNumSubresources));

    uint64 destTotalSizeBytes = 0u;
    myRenderer.GetDevice()->GetCopyableFootprints(&destDesc, aFirstSubresourceIndex, aNumSubresources, 0u, destLayouts, destRowNums, destRowSizesByte, &destTotalSizeBytes);

    // Prepare a temporary buffer that contains all subresource data in the expected form (i.e. respecting the dest data layout)
    uint8* tempBufferDataPtr;
    if (S_OK != aStagingResource->Map(0, nullptr, reinterpret_cast<void**>(&tempBufferDataPtr)))
      return;

    for (uint32 i = 0u; i < aNumSubresources; ++i)
    {
      D3D12_MEMCPY_DEST dest;
      dest.pData = tempBufferDataPtr + destLayouts[i].Offset;
      dest.RowPitch = destLayouts[i].Footprint.RowPitch;
      dest.SlicePitch = destLayouts[i].Footprint.RowPitch * destRowNums[i];
      locMemcpySubresourceRows(&dest, &someSubresourceDatas[i], destRowSizesByte[i], destRowNums[i], destLayouts[i].Footprint.Depth);
    }
    aStagingResource->Unmap(0, nullptr);

    // Copy from the temp staging buffer to the destination resource (could be buffer or texture)
    if (destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
      myCommandList->CopyBufferRegion(aDestResource, 0, aStagingResource, destLayouts[0].Offset, destLayouts[0].Footprint.Width);
    }
    else
    {
      for (uint32 i = 0u; i < aNumSubresources; ++i)
      {
        D3D12_TEXTURE_COPY_LOCATION destCopyLocation;
        destCopyLocation.pResource = aDestResource;
        destCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destCopyLocation.SubresourceIndex = aFirstSubresourceIndex + i;

        D3D12_TEXTURE_COPY_LOCATION srcCopyLocation;
        srcCopyLocation.pResource = aStagingResource;
        srcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcCopyLocation.PlacedFootprint = destLayouts[i];
        myCommandList->CopyTextureRegion(&destCopyLocation, 0u, 0u, 0u, &srcCopyLocation, nullptr);
      }
    }
  }
//---------------------------------------------------------------------------//
 void RenderContextDX12::CopyResource(GpuResourceDX12* aDestResource, GpuResourceDX12* aSrcResource)
 {
   D3D12_RESOURCE_STATES oldDestState = aDestResource->GetUsageState();
   D3D12_RESOURCE_STATES oldSrcState = aSrcResource->GetUsageState();

   TransitionResource(aDestResource, D3D12_RESOURCE_STATE_COPY_DEST);
   TransitionResource(aSrcResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
   KickoffResourceBarriers();

   myCommandList->CopyResource(aDestResource->GetResource(), aSrcResource->GetResource());

   TransitionResource(aDestResource, oldDestState);
   TransitionResource(aSrcResource, oldSrcState);
   KickoffResourceBarriers();
 }
//---------------------------------------------------------------------------//
  void RenderContextDX12::InitBufferData(GpuBufferDX12* aBuffer, void* aDataPtr)
  {
    RendererDX12* renderer = Fancy::GetRenderer();
    
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    const D3D12_RESOURCE_DESC& resourceDesc = aBuffer->GetResource()->GetDesc();

    ComPtr<ID3D12Resource> uploadResource;
    CheckD3Dcall(renderer->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, 
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* mappedBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &mappedBufferPtr));
    memcpy(mappedBufferPtr, aDataPtr, aBuffer->getTotalSizeBytes());
    uploadResource->Unmap(0, nullptr);

    RenderContext* initContext = RenderContext::AllocateContext();
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    initContext->myCommandList->CopyResource(aBuffer->GetResource(), uploadResource.Get());
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);
    
    initContext->ExecuteAndReset(true);
    
    RenderContext::FreeContext(initContext);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::InitTextureData(TextureDX12* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas)
  {
    RendererDX12* renderer = Fancy::GetRenderer();
    ID3D12Device* device = renderer->GetDevice();
    const D3D12_RESOURCE_DESC& resourceDesc = aTexture->GetResource()->GetDesc();

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

    ComPtr<ID3D12Resource> stagingBuffer;
    
    CheckD3Dcall(device->CreateCommittedResource(
      &HeapProps, D3D12_HEAP_FLAG_NONE,
      &BufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&stagingBuffer)));

    ASSERT(aTexture->getParameters().u16Depth <= 1u, "The code below might not work for 3D textures");

    D3D12_SUBRESOURCE_DATA* subDatas = static_cast<D3D12_SUBRESOURCE_DATA*>(alloca(sizeof(D3D12_SUBRESOURCE_DATA) * aNumUploadDatas));
    for (uint32 i = 0u; i < aNumUploadDatas; ++i)
    {
      subDatas[i].pData = someUploadDatas[i].myData;
      subDatas[i].SlicePitch = someUploadDatas[i].mySliceSizeBytes;
      subDatas[i].RowPitch = someUploadDatas[i].myRowSizeBytes;
    }

    RenderContext* uploadContext = RenderContext::AllocateContext();
    D3D12_RESOURCE_STATES oldUsageState = aTexture->GetUsageState();
    uploadContext->TransitionResource(aTexture, D3D12_RESOURCE_STATE_COPY_DEST, true);
    uploadContext->UpdateSubresources(aTexture->GetResource(), stagingBuffer.Get(), 0u, aNumUploadDatas, subDatas);
    uploadContext->TransitionResource(aTexture, oldUsageState, true);

    uploadContext->ExecuteAndReset(true);

    RenderContext::FreeContext(uploadContext);
  }
//---------------------------------------------------------------------------//
#pragma region Pipeline Apply
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyViewport()
  {
    if (!myViewportDirty)
      return;

    myViewportDirty = false;

    D3D12_VIEWPORT viewport = {0u};
    viewport.TopLeftX = myViewportParams.x;
    viewport.TopLeftY = myViewportParams.y;
    viewport.Width = myViewportParams.z;
    viewport.Height = myViewportParams.w;

    D3D12_RECT rect = { 0u };
    rect.left = viewport.TopLeftX;
    rect.top = viewport.TopLeftY;
    rect.right = viewport.Width;
    rect.bottom = viewport.Height;

    myCommandList->RSSetViewports(1u, &viewport);
    myCommandList->RSSetScissorRects(1u, &rect);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyRootSignature()
  {
    if (!myRootSignatureDirty)
      return;

    myCommandList->SetGraphicsRootSignature(myRootSignature);
    myRootSignatureDirty = false;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyRenderTargets()
  {
    if (!myRenderTargetsDirty)
      return;

    myRenderTargetsDirty = false;

    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[Rendering::Constants::kMaxNumRenderTargets];
    uint32 numRtsToSet = 0u;

    for (uint32 i = 0u; i < Rendering::Constants::kMaxNumRenderTargets; ++i)
    {
      Texture* rt = myRenderTargets[i];

      if (rt != nullptr)
        rtDescriptors[numRtsToSet++] = rt->GetRtv().myCpuHandle;
    }

    if (myDepthStencilTarget) 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &myDepthStencilTarget->GetDsv().myCpuHandle); 
    else 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyPipelineState()
  {
    if (!myPipelineState.myIsDirty)
      return;

    myPipelineState.myIsDirty = false;

    uint requestedHash = myPipelineState.getHash();

    ID3D12PipelineState* pso = nullptr;

    auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = myPipelineState.GetNativePSOdesc();
      HRESULT result = myRenderer.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT_M(result == S_OK, "Error creating graphics PSO");

      ourPSOcache[requestedHash] = pso;
    }
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyResourceState()
  {
    if (myResourceState.myDirtyFlags == 0u)
      return;

    myResourceState.myDirtyFlags = 0u;

    if (myPipelineState.myGpuProgramPipeline == nullptr)
      return;  // Nothing to set if we don't have shaders

    // Current strategy: The descriptors of all resources are dynamically copied into a 
    // a dynamic shader-visible heap, which is set as a descriptor table to the commandlist

    // This assumes a fixed ordering of CBVs before SRVs in the shader... TODO: Move this logic to higher-level code

    for (uint32 i = 0u; i < myResourceState.myConstantBufferRebindCount; ++i)
    {
      if (myResourceState.myConstantBuffers[i] != nullptr)
        myCommandList->SetGraphicsRootConstantBufferView(i, myResourceState.myConstantBuffers[i]->GetGpuVirtualAddress());
    }

    DescriptorDX12* srvDescriptorsToSet = 
      static_cast<DescriptorDX12*>(alloca(sizeof(DescriptorDX12) * myResourceState.myReadTexturesRebindCount));

    uint32 numSrvDescriptorsToSet = 0u;

    for (uint32 i = 0u; i < myResourceState.myReadTexturesRebindCount; ++i)
    {
      if (myResourceState.myReadTextures[i] != nullptr)
        srvDescriptorsToSet[numSrvDescriptorsToSet++] = myResourceState.myReadTextures[i]->GetSrv();
    }

    DescriptorDX12* samplerDescriptorsToSet =
      (DescriptorDX12*)alloca(sizeof(DescriptorDX12) * myResourceState.myTextureSamplerRebindCount);
    uint32 numSamplerDescriptorsToSet = 0u;

    for (uint32 i = 0u; i < myResourceState.myTextureSamplerRebindCount; ++i)
    {
      if (myResourceState.myTextureSamplers[i] != nullptr)
        samplerDescriptorsToSet[numSamplerDescriptorsToSet++] = myResourceState.myTextureSamplers[i]->GetDescriptor();
    }

    DescriptorHeapPoolDX12* heapPool = myRenderer.GetDescriptorHeapPool();

    if (numSrvDescriptorsToSet > 0u)
    {
      DescriptorHeapDX12* dynamicHeap = heapPool->AllocateDynamicHeap(numSrvDescriptorsToSet, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

      D3D12_CPU_DESCRIPTOR_HANDLE destHandle = dynamicHeap->GetCpuHeapStart();
      for (uint32 i = 0u; i < numSrvDescriptorsToSet; ++i)
      {
        myRenderer.GetDevice()->CopyDescriptorsSimple(1, destHandle,
          srvDescriptorsToSet[i].myCpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        destHandle.ptr += dynamicHeap->GetHandleIncrementSize();
      }

      SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, dynamicHeap);
      myCommandList->SetGraphicsRootDescriptorTable(6, dynamicHeap->GetGpuHeapStart());
    }

    //if (numSamplerDescriptorsToSet > 0u)
    //{
    //  DescriptorHeapDX12* dynamicHeap = heapPool->AllocateDynamicHeap(numSamplerDescriptorsToSet, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    //
    //  D3D12_CPU_DESCRIPTOR_HANDLE destHandle = dynamicHeap->GetCpuHeapStart();
    //  for (uint32 i = 0u; i < numSamplerDescriptorsToSet; ++i)
    //  {
    //    myRenderer.GetDevice()->CopyDescriptorsSimple(1, destHandle,
    //      samplerDescriptorsToSet[i].GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    //
    //    destHandle.ptr += dynamicHeap->GetHandleIncrementSize();
    //  }
    //
    //  SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, dynamicHeap);
    //  myCommandList->SetGraphicsRootDescriptorTable(0, dynamicHeap->GetGpuHeapStart());
    //}

    myResourceState.myDirtyFlags = 0u;
  }
//---------------------------------------------------------------------------//
#pragma endregion 


//---------------------------------------------------------------------------//
} } }
