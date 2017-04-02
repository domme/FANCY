#include "FancyCorePrerequisites.h"

#include <unordered_set>

#include "RenderContextDX12.h"
#include "MathUtil.h"
#include "AdapterDX12.h"
#include "GpuProgram.h"
#include "Renderer.h"
#include "DescriptorHeapPoolDX12.h"
#include "GpuBuffer.h"
#include "Texture.h"
#include "GeometryData.h"
#include "GpuProgramPipeline.h"
#include "ShaderResourceInterface.h"
#include "GpuResource.h"
#include "Descriptor.h"
#include "BlendState.h"
#include "DepthStencilState.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GraphicsPipelineState::GraphicsPipelineState()
    : myFillMode(FillMode::SOLID)
    , myCullMode(CullMode::BACK)
    , myWindingOrder(WindingOrder::CCW)
    , myNumRenderTargets(0u)
    , myDSVformat(DataFormat::UNKNOWN)
    , myIsDirty(true)
  {
    myDepthStencilState = RenderCore::CreateDepthStencilState(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    myBlendState = RenderCore::CreateBlendState(BlendStateDesc::GetDefaultSolid());
  }
  //---------------------------------------------------------------------------//
  uint GraphicsPipelineState::getHash()
  {
    uint hash = 0u;
    MathUtil::hash_combine(hash, static_cast<uint>(myFillMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myCullMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myWindingOrder));
    MathUtil::hash_combine(hash, myDepthStencilState->GetHash());
    MathUtil::hash_combine(hash, myBlendState->GetHash());
    MathUtil::hash_combine(hash, myGpuProgramPipeline->GetHash());

    if (myGpuProgramPipeline != nullptr)
      MathUtil::hash_combine(hash, myGpuProgramPipeline->GetShaderByteCodeHash());

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }
  //---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineState::GetNativePSOdesc()
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
    psoDesc.pRootSignature = myGpuProgramPipeline->GetRootSignature();

                                       // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = myBlendState->getAlphaToCoverageEnabled();
    blendDesc.IndependentBlendEnable = myBlendState->getBlendStatePerRT();
    uint rtCount = blendDesc.IndependentBlendEnable ? Constants::kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));

      rtBlendDesc.BlendEnable = myBlendState->myBlendEnabled[rt];
      rtBlendDesc.BlendOp = Adapter::toNativeType(myBlendState->myBlendOp[rt]);
      rtBlendDesc.BlendOpAlpha = Adapter::toNativeType(myBlendState->myBlendOpAlpha[rt]);
      rtBlendDesc.DestBlend = Adapter::toNativeType(myBlendState->myDestBlend[rt]);
      rtBlendDesc.DestBlendAlpha = Adapter::toNativeType(myBlendState->myDestBlendAlpha[rt]);

      // FEATURE: Add support for LogicOps?
      rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
      rtBlendDesc.LogicOpEnable = false;

      if (myBlendState->myRTwriteMask[rt] & 0xFFFFFF > 0u)
      {
        rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      }
      else
      {
        const bool red = (myBlendState->myRTwriteMask[rt] & 0xFF000000) > 0u;
        const bool green = (myBlendState->myRTwriteMask[rt] & 0x00FF0000) > 0u;
        const bool blue = (myBlendState->myRTwriteMask[rt] & 0x0000FF00) > 0u;
        const bool alpha = (myBlendState->myRTwriteMask[rt] & 0x000000FF) > 0u;
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
    dsState.DepthEnable = myDepthStencilState->myDepthTestEnabled;
    dsState.DepthWriteMask = myDepthStencilState->myDepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    dsState.DepthFunc = Adapter::toNativeType(myDepthStencilState->myDepthCompFunc);
    dsState.StencilEnable = myDepthStencilState->myStencilEnabled;
    dsState.StencilReadMask = static_cast<uint8>(myDepthStencilState->myStencilReadMask);
    dsState.StencilWriteMask = static_cast<uint8>(myDepthStencilState->myStencilWriteMask[0u]);
    // FrontFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.FrontFace;
      uint faceIdx = static_cast<uint>(FaceType::FRONT);
      faceDesc.StencilFunc = Adapter::toNativeType(myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(myDepthStencilState->myStencilPassOp[faceIdx]);
    }
    // BackFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.BackFace;
      uint faceIdx = static_cast<uint>(FaceType::BACK);
      faceDesc.StencilFunc = Adapter::toNativeType(myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(myDepthStencilState->myStencilPassOp[faceIdx]);
    }

    // INPUT LAYOUT

    if (myGpuProgramPipeline != nullptr &&
        myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX] != nullptr)
    {
      const GpuProgram* vertexShader =
        myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX].get();

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
      psoDesc.RTVFormats[i] = RenderCore::GetFormat(myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = RenderCore::GetFormat(myDSVformat);

    // NODE MASK
    psoDesc.NodeMask = 0u;

    return psoDesc;
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> RenderContextDX12::ourPSOcache;
//---------------------------------------------------------------------------// 
  RenderContextDX12::RenderContextDX12()
    : CommandContextBaseDX12(CommandListType::Graphics)
    , myViewportParams(0, 0, 1, 1)
    , myViewportDirty(true)
    , myRenderTargetsDirty(true)
    , myDepthStencilTarget(nullptr)
  {
    RenderContextDX12::Reset_Internal();
  }
//---------------------------------------------------------------------------//
  RenderContextDX12::~RenderContextDX12()
  {
  }  
//---------------------------------------------------------------------------//
  void RenderContextDX12::Reset_Internal()
  {
    CommandContextBaseDX12::Reset_Internal();

    myGraphicsPipelineState = GraphicsPipelineState();
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));
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
  void RenderContextDX12::SetBlendState(std::shared_ptr<BlendState> aBlendState)
  {
    SharedPtr<BlendState> stateToSet =
      aBlendState ? aBlendState : RenderCore::GetDefaultBlendState();

    GraphicsPipelineState& state = myGraphicsPipelineState;
    const uint requestedHash = stateToSet->GetHash();

    if (state.myBlendState->GetHash() == requestedHash)
      return;

    state.myBlendState = stateToSet;
    state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::SetDepthStencilState(std::shared_ptr<DepthStencilState> aDepthStencilState)
  {
    SharedPtr<DepthStencilState> stateToSet = 
      aDepthStencilState ? aDepthStencilState : RenderCore::GetDefaultDepthStencilState();

      GraphicsPipelineState& state = myGraphicsPipelineState;
      uint requestedHash = stateToSet->GetHash();

      if (state.myDepthStencilState->GetHash() == requestedHash)
        return;

      state.myDepthStencilState = stateToSet;
      state.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setFillMode(const FillMode eFillMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setCullMode(const CullMode eCullMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setWindingOrder(const WindingOrder eWindingOrder)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
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

    myGraphicsPipelineState.myDSVformat = pDStexture->getParameters().eFormat;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
  {
    if (myRenderTargets[u8RenderTargetIndex] == pRTTexture)
      return;

    myRenderTargets[u8RenderTargetIndex] = pRTTexture;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myNumRenderTargets = glm::max(myGraphicsPipelineState.myNumRenderTargets, (uint8) (u8RenderTargetIndex + 1));
    myGraphicsPipelineState.myRTVformats[u8RenderTargetIndex] = pRTTexture->getParameters().eFormat;
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::removeAllRenderTargets()
  {
    memset(myRenderTargets, 0, sizeof(myRenderTargets));
    myDepthStencilTarget = nullptr;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myNumRenderTargets = 0u;
    for (uint32 i = 0u; i < ARRAY_LENGTH(myGraphicsPipelineState.myRTVformats); ++i)
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;

    myGraphicsPipelineState.myDSVformat = DataFormat::NONE;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
    myCommandList->SetGraphicsRootShaderResourceView(aRegisterIndex, aTexture->GetGpuVirtualAddress());    
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
    myCommandList->SetGraphicsRootUnorderedAccessView(aRegisterIndex, aTexture->GetGpuVirtualAddress());
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
    myCommandList->SetGraphicsRootShaderResourceView(aRegisterIndex, aBuffer->GetGpuVirtualAddress());
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
    myCommandList->SetGraphicsRootConstantBufferView(aRegisterIndex, aConstantBuffer->GetGpuVirtualAddress());
  }  
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex)
  {
    ASSERT(myRootSignature != nullptr);
    ASSERT(aResourceCount > 0u);

    D3D12_DESCRIPTOR_HEAP_TYPE heapType = someResources[0].myHeapType;
    DescriptorHeapDX12* dynamicHeap = myDynamicShaderVisibleHeaps[heapType];
    
    if (dynamicHeap == nullptr)
    {
      dynamicHeap = RenderCore::AllocateDynamicDescriptorHeap(aResourceCount, heapType);
      SetDescriptorHeap(heapType, dynamicHeap);
    }

    uint32 startOffset = dynamicHeap->GetNumAllocatedDescriptors();
    for (uint32 i = 0u; i < aResourceCount; ++i)
    {
      DescriptorDX12 destDescriptor = dynamicHeap->AllocateDescriptor();
    
      RenderCore::GetDevice()->CopyDescriptorsSimple(1, destDescriptor.myCpuHandle,
        someResources[i].myCpuHandle, heapType);
    }
    
    myCommandList->SetGraphicsRootDescriptorTable(aRegisterIndex, dynamicHeap->GetDescriptor(startOffset).myGpuHandle);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    if (myGraphicsPipelineState.myGpuProgramPipeline != aGpuProgramPipeline)
    {
      myGraphicsPipelineState.myGpuProgramPipeline = aGpuProgramPipeline;
      myGraphicsPipelineState.myIsDirty = true;
    }

    if (myRootSignature != aGpuProgramPipeline->GetRootSignature())
    {
      myRootSignature = aGpuProgramPipeline->GetRootSignature();
      myCommandList->SetGraphicsRootSignature(myRootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::renderGeometry(const Geometry::GeometryData* pGeometry)
  {
    ApplyViewport();
    ApplyRenderTargets();
    ApplyPipelineState();

    myCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    myCommandList->IASetVertexBuffers(0, 1, &pGeometry->getVertexBuffer()->GetVertexBufferView());
    myCommandList->IASetIndexBuffer(&pGeometry->getIndexBuffer()->GetIndexBufferView());
    myCommandList->DrawIndexedInstanced(pGeometry->getNumIndices(), 1, 0, 0, 0);
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
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT rect = { 0u };
    rect.left = viewport.TopLeftX;
    rect.top = viewport.TopLeftY;
    rect.right = viewport.Width;
    rect.bottom = viewport.Height;
    

    myCommandList->RSSetViewports(1u, &viewport);
    myCommandList->RSSetScissorRects(1u, &rect);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyRenderTargets()
  {
    if (!myRenderTargetsDirty)
      return;

    myRenderTargetsDirty = false;

    D3D12_CPU_DESCRIPTOR_HANDLE rtDescriptors[Rendering::Constants::kMaxNumRenderTargets];
    GpuResource* rtResources[Rendering::Constants::kMaxNumRenderTargets];
    uint32 numRtsToSet = 0u;

    for (uint32 i = 0u; i < Rendering::Constants::kMaxNumRenderTargets; ++i)
    {
      Texture* rt = myRenderTargets[i];

      if (rt != nullptr)
      {
        rtResources[numRtsToSet] = rt;
        rtDescriptors[numRtsToSet] = rt->GetRtv().myCpuHandle;
        ++numRtsToSet;
      }
    }

    for (uint32 i = 0u; i < numRtsToSet; ++i)
    {
      TransitionResource(rtResources[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    if (myDepthStencilTarget)
      TransitionResource(myDepthStencilTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    KickoffResourceBarriers();

    if (myDepthStencilTarget) 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &myDepthStencilTarget->GetDsv().myCpuHandle); 
    else 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    uint requestedHash = myGraphicsPipelineState.getHash();

    ID3D12PipelineState* pso = nullptr;

    auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = myGraphicsPipelineState.GetNativePSOdesc();
      HRESULT result = RenderCore::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT(result == S_OK, "Error creating graphics PSO");

      ourPSOcache[requestedHash] = pso;
    }
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
#pragma endregion 
} } }
