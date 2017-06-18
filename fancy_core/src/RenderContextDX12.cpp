#include "FancyCorePrerequisites.h"

#include <malloc.h>

#include "RenderContextDX12.h"
#include "AdapterDX12.h"
#include "GpuBuffer.h"
#include "Texture.h"
#include "GeometryData.h"
#include "GpuProgramPipeline.h"
#include "Descriptor.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RenderCore.h"
#include "GpuProgramDX12.h"
#include "TextureDX12.h"
#include "GpuBufferDX12.h"
#include "RenderCore_PlatformDX12.h"
#include "ShaderResourceInterfaceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  std::unordered_map<uint, ID3D12PipelineState*> RenderContextDX12::ourPSOcache;
//---------------------------------------------------------------------------// 
  RenderContextDX12::RenderContextDX12()
    : CommandContextDX12(CommandListType::Graphics)
  {
    RenderContextDX12::Reset_Internal();
  }
//---------------------------------------------------------------------------//
  RenderContextDX12::~RenderContextDX12()
  {
    RenderContextDX12::Reset_Internal();
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::Reset()
  {
    Reset_Internal();
  }
//---------------------------------------------------------------------------//
  uint64 RenderContextDX12::ExecuteAndReset(bool aWaitForCompletion)
  {
    return ExecuteAndReset_Internal(aWaitForCompletion);
  }
  //---------------------------------------------------------------------------//
  D3D12_GRAPHICS_PIPELINE_STATE_DESC RenderContextDX12::GetNativePSOdesc(const GraphicsPipelineState& aState)
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.DS, &psoDesc.HS, &psoDesc.GS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == (uint)ShaderStage::NUM_NO_COMPUTE);

    if (aState.myGpuProgramPipeline != nullptr)
    {
      for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
      {
        if (nullptr == aState.myGpuProgramPipeline->myGpuPrograms[i])
          continue;

        const GpuProgramDX12* shaderDx12 = static_cast<const GpuProgramDX12*>(aState.myGpuProgramPipeline->myGpuPrograms[i].get());

        (*shaderDescs[i]) = shaderDx12->getNativeByteCode();
      }
    }

    // ROOT SIGNATURE
    const ShaderResourceInterfaceDX12* sriDx12 = static_cast<const ShaderResourceInterfaceDX12*>(aState.myGpuProgramPipeline->myResourceInterface);
    psoDesc.pRootSignature = sriDx12->myRootSignature.Get();

    // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = aState.myBlendState->getAlphaToCoverageEnabled();
    blendDesc.IndependentBlendEnable = aState.myBlendState->getBlendStatePerRT();
    uint rtCount = blendDesc.IndependentBlendEnable ? Constants::kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));

      rtBlendDesc.BlendEnable = aState.myBlendState->myBlendEnabled[rt];
      rtBlendDesc.BlendOp = Adapter::toNativeType(aState.myBlendState->myBlendOp[rt]);
      rtBlendDesc.BlendOpAlpha = Adapter::toNativeType(aState.myBlendState->myBlendOpAlpha[rt]);
      rtBlendDesc.DestBlend = Adapter::toNativeType(aState.myBlendState->myDestBlend[rt]);
      rtBlendDesc.DestBlendAlpha = Adapter::toNativeType(aState.myBlendState->myDestBlendAlpha[rt]);

      // FEATURE: Add support for LogicOps?
      rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
      rtBlendDesc.LogicOpEnable = false;

      if (aState.myBlendState->myRTwriteMask[rt] & 0xFFFFFF > 0u)
      {
        rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      }
      else
      {
        const bool red = (aState.myBlendState->myRTwriteMask[rt] & 0xFF000000) > 0u;
        const bool green = (aState.myBlendState->myRTwriteMask[rt] & 0x00FF0000) > 0u;
        const bool blue = (aState.myBlendState->myRTwriteMask[rt] & 0x0000FF00) > 0u;
        const bool alpha = (aState.myBlendState->myRTwriteMask[rt] & 0x000000FF) > 0u;
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
    rasterizerDesc.FillMode = Adapter::toNativeType(aState.myFillMode);
    rasterizerDesc.CullMode = Adapter::toNativeType(aState.myCullMode);
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.FrontCounterClockwise = aState.myWindingOrder == WindingOrder::CCW;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = false;

    // DEPTH STENCIL STATE
    D3D12_DEPTH_STENCIL_DESC& dsState = psoDesc.DepthStencilState;
    dsState.DepthEnable = aState.myDepthStencilState->myDepthTestEnabled;
    dsState.DepthWriteMask = aState.myDepthStencilState->myDepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    dsState.DepthFunc = Adapter::toNativeType(aState.myDepthStencilState->myDepthCompFunc);
    dsState.StencilEnable = aState.myDepthStencilState->myStencilEnabled;
    dsState.StencilReadMask = static_cast<uint8>(aState.myDepthStencilState->myStencilReadMask);
    dsState.StencilWriteMask = static_cast<uint8>(aState.myDepthStencilState->myStencilWriteMask[0u]);
    // FrontFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.FrontFace;
      uint faceIdx = static_cast<uint>(FaceType::FRONT);
      faceDesc.StencilFunc = Adapter::toNativeType(aState.myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilPassOp[faceIdx]);
    }
    // BackFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.BackFace;
      uint faceIdx = static_cast<uint>(FaceType::BACK);
      faceDesc.StencilFunc = Adapter::toNativeType(aState.myDepthStencilState->myStencilCompFunc[faceIdx]);
      faceDesc.StencilDepthFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilDepthFailOp[faceIdx]);
      faceDesc.StencilFailOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilFailOp[faceIdx]);
      faceDesc.StencilPassOp = Adapter::toNativeType(aState.myDepthStencilState->myStencilPassOp[faceIdx]);
    }

    // INPUT LAYOUT

    if (aState.myGpuProgramPipeline != nullptr &&
      aState.myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX] != nullptr)
    {
      const GpuProgramDX12* vertexShader =
        static_cast<const GpuProgramDX12*>(aState.myGpuProgramPipeline->myGpuPrograms[(uint32)ShaderStage::VERTEX].get());

      D3D12_INPUT_LAYOUT_DESC& inputLayout = psoDesc.InputLayout;
      inputLayout.NumElements = vertexShader->GetNumNativeInputElements();
      inputLayout.pInputElementDescs = vertexShader->GetNativeInputElements();
    }

    // IB STRIP CUT VALUE
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // TOPOLOGY TYPE
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // NUM RENDER TARGETS
    psoDesc.NumRenderTargets = aState.myNumRenderTargets;

    // RTV-FORMATS
    for (uint i = 0u; i < aState.myNumRenderTargets; ++i)
    {
      psoDesc.RTVFormats[i] = RenderCore_PlatformDX12::GetFormat(aState.myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = RenderCore_PlatformDX12::GetFormat(aState.myDSVformat);

    // NODE MASK
    psoDesc.NodeMask = 0u;

    return psoDesc;
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::Reset_Internal()
  {
    CommandContextDX12::Reset_Internal();

    myGraphicsPipelineState = GraphicsPipelineState();
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ClearRenderTarget(Texture* aTexture, const float* aColor)
  {
    ClearRenderTarget_Internal(aTexture, aColor);
  }
  //---------------------------------------------------------------------------//
  void RenderContextDX12::ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags) const
  {
    ClearDepthStencilTarget_Internal(aTexture, aDepthClear, aStencilClear, someClearFlags);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::BindResource(const GpuResource* aResource, ResourceBindingType aBindingType, uint32 aRegisterIndex) const
  {
    ASSERT(myRootSignature != nullptr);
 
    const GpuResourceDX12* resource = CastGpuResourceDX12(aResource);
    ASSERT(resource != nullptr);

    const uint64 gpuVirtualAddress = resource->GetGpuVirtualAddress();
    
    switch(aBindingType)
    {
      case ResourceBindingType::SIMPLE: { myCommandList->SetGraphicsRootShaderResourceView(aRegisterIndex, gpuVirtualAddress); break; }
      case ResourceBindingType::READ_WRITE: { myCommandList->SetGraphicsRootUnorderedAccessView(aRegisterIndex, gpuVirtualAddress); break; }
      case ResourceBindingType::CONSTANT_BUFFER: { myCommandList->SetGraphicsRootConstantBufferView(aRegisterIndex, gpuVirtualAddress); break; }
      case ResourceBindingType::RENDER_TARGET:
      case ResourceBindingType::DEPTH_STENCIL_TARGET:
      default: { ASSERT(false); break; }
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::BindResourceSet(const GpuResource** someResources, ResourceBindingType* someBindingTypes, uint32 aResourceCount, uint32 aRegisterIndex)
  {
    ASSERT(myRootSignature != nullptr);

    const GpuResourceDX12** resources = static_cast<const GpuResourceDX12**>(alloca(sizeof(const GpuResourceDX12*) * aResourceCount));
    for (uint32 i = 0u; i < aResourceCount; ++i)
    {
      resources[i] = CastGpuResourceDX12(someResources[i]);
      ASSERT(resources[i] != nullptr);
    }

    DescriptorDX12* descriptors = static_cast<DescriptorDX12*>(alloca(sizeof(DescriptorDX12) * aResourceCount));
    for (uint32 i = 0u; i < aResourceCount; ++i)
    {
      switch (someBindingTypes[i])
      {
        case ResourceBindingType::SIMPLE: { const DescriptorDX12* desc = resources[i]->GetSrv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::READ_WRITE: { const DescriptorDX12* desc = resources[i]->GetUav(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::RENDER_TARGET: { const DescriptorDX12* desc = resources[i]->GetRtv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::DEPTH_STENCIL_TARGET: { const DescriptorDX12* desc = resources[i]->GetDsv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        case ResourceBindingType::CONSTANT_BUFFER: { const DescriptorDX12* desc = resources[i]->GetCbv(); ASSERT(desc != nullptr); descriptors[i] = *desc; break; }
        default: { ASSERT(false); break; };
      }
    }

    DescriptorDX12 dynamicRangeStartDescriptor = CopyDescriptorsToDynamicHeapRange(descriptors, aResourceCount);
    myCommandList->SetGraphicsRootDescriptorTable(aRegisterIndex, dynamicRangeStartDescriptor.myGpuHandle);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    RenderContext::SetGpuProgramPipeline(aGpuProgramPipeline);

    const ShaderResourceInterfaceDX12* sriDx12 = 
      static_cast<const ShaderResourceInterfaceDX12*>(aGpuProgramPipeline->myResourceInterface);

    if (myRootSignature != sriDx12->myRootSignature.Get())
    {
      myRootSignature = sriDx12->myRootSignature.Get();
      myCommandList->SetGraphicsRootSignature(myRootSignature);
    }
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::RenderGeometry(const Geometry::GeometryData* pGeometry)
  {
    ApplyViewport();
    ApplyRenderTargets();
    ApplyPipelineState();

    const GpuBufferDX12* vertexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getVertexBuffer());
    const GpuBufferDX12* indexBufferDx12 = static_cast<const GpuBufferDX12*>(pGeometry->getIndexBuffer());

    myCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    myCommandList->IASetVertexBuffers(0, 1, &vertexBufferDx12->GetVertexBufferView());
    myCommandList->IASetIndexBuffer(&indexBufferDx12->GetIndexBufferView());
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
    GpuResourceDX12* rtResources[Rendering::Constants::kMaxNumRenderTargets];
    uint32 numRtsToSet = 0u;

    for (uint32 i = 0u; i < Rendering::Constants::kMaxNumRenderTargets; ++i)
    {
      TextureDX12* rt = static_cast<TextureDX12*>(myRenderTargets[i]);
      ASSERT(rt->GetRtv() != nullptr);

      if (rt != nullptr)
      {
        rtResources[numRtsToSet] = rt;
        rtDescriptors[numRtsToSet] = rt->GetRtv()->myCpuHandle;
        ++numRtsToSet;
      }
    }

    for (uint32 i = 0u; i < numRtsToSet; ++i)
    {
      TransitionResource(rtResources[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    TextureDX12* dsvTargetDx12 = static_cast<TextureDX12*>(myDepthStencilTarget);
    ASSERT(dsvTargetDx12->GetDsv() != nullptr);

    if (dsvTargetDx12)
      TransitionResource(dsvTargetDx12, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    KickoffResourceBarriers();

    if (myDepthStencilTarget) 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, &dsvTargetDx12->GetDsv()->myCpuHandle);
    else 
      myCommandList->OMSetRenderTargets(numRtsToSet, rtDescriptors, false, nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderContextDX12::ApplyPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    uint requestedHash = myGraphicsPipelineState.GetHash();

    ID3D12PipelineState* pso = nullptr;

    auto cachedPSOIter = ourPSOcache.find(requestedHash);
    if (cachedPSOIter != ourPSOcache.end())
    {
      pso = cachedPSOIter->second;
    }
    else
    {
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc = GetNativePSOdesc(myGraphicsPipelineState);
      HRESULT result = RenderCore::GetPlatformDX12()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
      ASSERT(result == S_OK, "Error creating graphics PSO");

      ourPSOcache[requestedHash] = pso;
    }
    myCommandList->SetPipelineState(pso);
  }
//---------------------------------------------------------------------------//
#pragma endregion 
} } }
