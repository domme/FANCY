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
  namespace {
    std::vector<std::unique_ptr<RenderContextDX12>> locRenderContextPool;
    std::list<RenderContextDX12*> locAvailableRenderContexts;
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> RenderContextDX12::ourPSOcache;
//---------------------------------------------------------------------------//
  RenderContextDX12* RenderContextDX12::AllocateContext()
  {
    if (!locAvailableRenderContexts.empty())
    {
      RenderContextDX12* context = locAvailableRenderContexts.front();
      context->Reset();
      locAvailableRenderContexts.pop_front();
      return context;
    }

    locRenderContextPool.push_back(std::make_unique<RenderContextDX12>());
    return locRenderContextPool.back().get();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::FreeContext(RenderContextDX12* aContext)
  {
    if (std::find(locAvailableRenderContexts.begin(), locAvailableRenderContexts.end(), aContext)
      != locAvailableRenderContexts.end())
      return;

    locAvailableRenderContexts.push_back(aContext);
  }
//---------------------------------------------------------------------------//
  RenderContextDX12::RenderContextDX12()
    : myRenderer(*Fancy::GetRenderer())
    , myCommandAllocatorPool(*myRenderer.GetCommandAllocatorPool())
    , myPSOhash(0u)
    , myPSO(nullptr)
    , myViewportParams(0, 0, 1, 1)
    , myViewportDirty(true)
    , myRootSignature(nullptr)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myCpuVisibleAllocator(myRenderer, GpuDynamicAllocatorType::CpuWritable)
    , myGpuOnlyAllocator(myRenderer, GpuDynamicAllocatorType::GpuOnly)
    , myIsInRecordState(true)
  {
    memset(myDescriptorHeaps, 0u, sizeof(myDescriptorHeaps));

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
    , myPSOhash(0u)
    , myPSO(nullptr)
    , myViewportParams(0, 0, 1, 1)
    , myViewportDirty(true)
    , myRootSignature(nullptr)
    , myCommandList(nullptr)
    , myCommandAllocator(nullptr)
    , myCpuVisibleAllocator(myRenderer, GpuDynamicAllocatorType::CpuWritable)
    , myGpuOnlyAllocator(myRenderer, GpuDynamicAllocatorType::GpuOnly)
    , myIsInRecordState(true)
  {
    memset(myDescriptorHeaps, 0u, sizeof(myDescriptorHeaps));

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
    myPSO->Release();
    myPSO = nullptr;
    myCommandList->Release();
    myCommandList = nullptr;

    if (myCommandAllocator != nullptr)
      ReleaseAllocator(0u);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::Reset()
  {
    if (myIsInRecordState)
      return;
    
    ASSERT_M(nullptr == myCommandAllocator, "myIsInRecordState-flag out of sync");

    myCommandAllocator = myCommandAllocatorPool.GetNewAllocator();
    ASSERT(myCommandAllocator != nullptr);

    CheckD3Dcall(myCommandList->Reset(myCommandAllocator, myPSO));

    myIsInRecordState = true;
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
    ReleaseAllocator(fenceVal);

    if (aWaitForCompletion)
      myRenderer.WaitForFence(fenceVal);

    Reset();

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, DescriptorHeapDX12* aDescriptorHeap)
  {
    ID3D12DescriptorHeap* heap = aDescriptorHeap->GetHeap();

    if (myDescriptorHeaps[aHeapType] == heap)
      return;

    myDescriptorHeaps[aHeapType] = heap;
    ApplyDescriptorHeaps();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE aRTV, const float* aColor)
  {
    myCommandList->ClearRenderTargetView(aRTV, aColor, 0, nullptr);
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
    
    for(ID3D12DescriptorHeap* heap : myDescriptorHeaps)
      if (heap != nullptr)
        heapsToBind[numHeapsToBind++] = heap;
    
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
    PipelineState& state = myState;
    uint requestedHash = clBlendState.GetHash();

    if (state.myBlendState.GetHash() == requestedHash)
      return;

    state.myBlendState = clBlendState;
    state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setDepthStencilState(const DepthStencilState& aDepthStencilState)
  {
    PipelineState& state = myState;
    uint requestedHash = aDepthStencilState.GetHash();

    if (state.myDepthStencilState.GetHash() == requestedHash)
      return;

    state.myDepthStencilState = aDepthStencilState;
    state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setFillMode(const FillMode eFillMode)
  {
    PipelineState& state = myState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setCullMode(const CullMode eCullMode)
  {
    PipelineState& state = myState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setWindingOrder(const WindingOrder eWindingOrder)
  {
    PipelineState& state = myState;
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setDepthStencilRenderTarget(Texture* pDStexture)
  {

  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
  {

  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::removeAllRenderTargets()
  {
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetGraphicsRootSignature(ID3D12RootSignature* aRootSignature)
  {
    if (myRootSignature != aRootSignature)
      myCommandList->SetGraphicsRootSignature(aRootSignature);
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setReadTexture(const Texture* pTexture, const uint8 u8RegisterIndex)
  {
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
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setTextureSampler(const TextureSampler* pSampler, const uint8 u8RegisterIndex)
  {
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage)
  {
    PipelineState& state = myState;

    if (state.myShaderStages[(uint)eShaderStage] == pProgram)
      return;

    state.myShaderStages[(uint)eShaderStage] = pProgram;
    state.myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::renderGeometry(const Geometry::GeometryData* pGeometry)
  {
    applyPipelineState();
    applyViewport();
  }
//---------------------------------------------------------------------------//
  //void RenderContextDX12::CopySubresources(ID3D12Resource* aDestResource, ID3D12Resource* aSrcResource, uint aFirstSubresource, uint aSubResourceCount)
  //{
  //  
  //}
//---------------------------------------------------------------------------//
  namespace {
    void locMemcpySubresourceRows(const D3D12_MEMCPY_DEST* aDest, const D3D12_SUBRESOURCE_DATA* aSrc, size_t aRowStrideBytes, uint32 aNumRows, uint32 aNumSlices)
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
        destCopyLocation.PlacedFootprint = destLayouts[i];
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
    CheckD3Dcall(renderer->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, 
      &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource)));

    void* mappedBufferPtr;
    CheckD3Dcall(uploadResource->Map(0, nullptr, &mappedBufferPtr));
    memcpy(mappedBufferPtr, aDataPtr, aBuffer->getTotalSizeBytes());
    uploadResource->Unmap(0, nullptr);

    RenderContextDX12* initContext = RenderContextDX12::AllocateContext();
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    initContext->myCommandList->CopyResource(aBuffer->GetResource(), uploadResource.Get());
    initContext->TransitionResource(aBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);
    
    initContext->ExecuteAndReset(true);
    
    RenderContextDX12::FreeContext(initContext);
  }

//---------------------------------------------------------------------------//
  void RenderContextDX12::InitTextureData(TextureDX12* aTexture, const TextureUploadData* someUploadDatas, uint32 aNumUploadDatas)
  {
    RendererDX12* renderer = Fancy::GetRenderer();
    ID3D12Device* device = renderer->GetDevice();
    const D3D12_RESOURCE_DESC& resourceDesc = aTexture->GetResource()->GetDesc();

    uint64 requiredStagingBufferSize;
    device->GetCopyableFootprints(&resourceDesc, 0u, aNumUploadDatas, 0u, nullptr, nullptr, nullptr, &requiredStagingBufferSize);

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

    RenderContextDX12* uploadContext = RenderContextDX12::AllocateContext();
    D3D12_RESOURCE_STATES oldUsageState = aTexture->GetUsageState();
    uploadContext->TransitionResource(aTexture, D3D12_RESOURCE_STATE_COPY_DEST, true);
    uploadContext->UpdateSubresources(aTexture->GetResource(), stagingBuffer.Get(), 0u, aNumUploadDatas, subDatas);
    uploadContext->TransitionResource(aTexture, oldUsageState, true);

    uploadContext->ExecuteAndReset(true);

    RenderContextDX12::FreeContext(uploadContext);
  }
//---------------------------------------------------------------------------//
#pragma region Pipeline Apply
//---------------------------------------------------------------------------//
  void RenderContextDX12::applyViewport()
  {

  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::applyPipelineState()
  {
    PipelineState& requestedState = myState;

    if (!requestedState.myIsDirty)
      return;

    uint requestedHash = requestedState.getHash();

    ID3D12PipelineState* pso = nullptr;

    auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = requestedState.toNativePSOdesc();
      HRESULT result = myRenderer.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT_M(result == S_OK, "Error creating graphics PSO");

      ourPSOcache[requestedHash] = pso;
    }
    requestedState.myIsDirty = false;

    myCommandList->SetPipelineState(pso);
  }
  //---------------------------------------------------------------------------//
#pragma endregion 


//---------------------------------------------------------------------------//
} } }
