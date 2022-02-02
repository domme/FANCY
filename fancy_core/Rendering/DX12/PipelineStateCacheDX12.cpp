#include "fancy_core_precompile.h"

#include "Rendering/DepthStencilState.h"
#include "Rendering/CommandList.h"
#include "Rendering/BlendState.h"

#include "PipelineStateCacheDX12.h"

#include "AdapterDX12.h"
#include "ShaderDX12.h"
#include "ShaderPipelineDX12.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Priv_PipelineStateCacheDX12
  {
    const char* VertexAttributeSemanticToString(VertexAttributeSemantic aSemantic)
    {
      switch (aSemantic)
      {
      case VertexAttributeSemantic::POSITION: return "POSITION";
      case VertexAttributeSemantic::NORMAL: return "NORMAL";
      case VertexAttributeSemantic::TANGENT: return "TANGENT";
      case VertexAttributeSemantic::BINORMAL: return "BINORMAL";
      case VertexAttributeSemantic::TEXCOORD: return "TEXCOORD";
      case VertexAttributeSemantic::COLOR: return "COLOR";
      default:
        ASSERT(false); return "POSITION";
      }
    }
  }
//---------------------------------------------------------------------------//
  PipelineStateCacheDX12::~PipelineStateCacheDX12()
  {
    PipelineStateCacheDX12::Clear();
  }
//---------------------------------------------------------------------------//
  ID3D12PipelineState* PipelineStateCacheDX12::GetCreateGraphicsPSO(const GraphicsPipelineState& aState)
  {
    const uint64 hash = aState.GetHash();

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myGraphicsPsoCache.find(hash);
      if (it != myGraphicsPsoCache.end())
        return it->second;
    }

    ASSERT(aState.myNumRenderTargets <= RenderConstants::kMaxNumRenderTargets);
    ASSERT(aState.myNumRenderTargets > 0 || aState.myDSVformat != DataFormat::NONE);
    ASSERT(aState.myShaderPipeline != nullptr);

    // TODO: Remove all those memsets, they shouldn't be necessary

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0u, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    // SHADER BYTECODES
    D3D12_SHADER_BYTECODE* shaderDescs[]{ &psoDesc.VS, &psoDesc.PS, &psoDesc.GS, &psoDesc.HS, &psoDesc.DS };
    ASSERT(ARRAY_LENGTH(shaderDescs) == SHADERSTAGE_NUM_RASTERIZATION);

    if (aState.myShaderPipeline != nullptr)
    {
      for (uint i = 0u; i < SHADERSTAGE_NUM_RASTERIZATION; ++i)
      {
        if (nullptr == aState.myShaderPipeline->GetShader(i))
          continue;

        const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(aState.myShaderPipeline->GetShader(i));
        *shaderDescs[i] = shaderDx12->getNativeByteCode();
      }
    }

    // ROOT SIGNATURE
    psoDesc.pRootSignature = RenderCore::GetPlatformDX12()->GetRootSignature()->GetRootSignature();

    // BLEND DESC
    D3D12_BLEND_DESC& blendDesc = psoDesc.BlendState;
    const BlendStateProperties& blendProps = aState.myBlendState->GetProperties();

    memset(&blendDesc, 0u, sizeof(D3D12_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = blendProps.myAlphaToCoverageEnabled;
    blendDesc.IndependentBlendEnable = blendProps.myBlendStatePerRT;
    uint rtCount = blendDesc.IndependentBlendEnable ? RenderConstants::kMaxNumRenderTargets : 1u;
    for (uint rt = 0u; rt < rtCount; ++rt)
    {
      D3D12_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[rt];
      const BlendStateRenderTargetProperties& rtBlendProps = blendProps.myRendertargetProperties[rt];

      memset(&rtBlendDesc, 0u, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
      rtBlendDesc.BlendEnable = rtBlendProps.myBlendEnabled;

      rtBlendDesc.BlendOp = Adapter::toNativeType(rtBlendProps.myBlendOp);
      rtBlendDesc.BlendOpAlpha = rtBlendProps.myAlphaSeparateBlend ? Adapter::toNativeType(rtBlendProps.myBlendOpAlpha) : rtBlendDesc.BlendOp;

      rtBlendDesc.DestBlend = Adapter::toNativeType(rtBlendProps.myDstBlendFactor);
      rtBlendDesc.DestBlendAlpha = rtBlendProps.myAlphaSeparateBlend ? Adapter::toNativeType(rtBlendProps.myDstBlendAlphaFactor) : rtBlendDesc.DestBlend;

      rtBlendDesc.SrcBlend = Adapter::toNativeType(rtBlendProps.mySrcBlendFactor);
      rtBlendDesc.SrcBlendAlpha = rtBlendProps.myAlphaSeparateBlend ? Adapter::toNativeType(rtBlendProps.mySrcBlendAlphaFactor) : rtBlendDesc.SrcBlend;

      rtBlendDesc.LogicOpEnable = blendProps.myLogicOpEnabled;
      rtBlendDesc.LogicOp = RenderCore_PlatformDX12::ResolveLogicOp(blendProps.myLogicOp);

      const uint channelWriteMask = rtBlendProps.myColorChannelWriteMask;
      if ((channelWriteMask & 0xFFFFFF) > 0u)
      {
        rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
      }
      else
      {
        const bool red = (channelWriteMask & 0xFF000000) > 0u;
        const bool green = (channelWriteMask & 0x00FF0000) > 0u;
        const bool blue = (channelWriteMask & 0x0000FF00) > 0u;
        const bool alpha = (channelWriteMask & 0x000000FF) > 0u;
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
    const DepthStencilStateProperties& dsProps = aState.myDepthStencilState->GetProperties();

    dsState.DepthEnable = dsProps.myDepthTestEnabled;
    dsState.DepthWriteMask = dsProps.myDepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    dsState.DepthFunc = RenderCore_PlatformDX12::ResolveCompFunc(dsProps.myDepthCompFunc);
    dsState.StencilEnable = dsProps.myStencilEnabled;
    dsState.StencilReadMask = static_cast<uint8>(dsProps.myStencilReadMask);
    dsState.StencilWriteMask = static_cast<uint8>(dsProps.myStencilWriteMask);

    // FrontFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.FrontFace;
      const DepthStencilFaceProperties& faceProps = dsProps.myFrontFace;
      faceDesc.StencilFunc = RenderCore_PlatformDX12::ResolveCompFunc(faceProps.myStencilCompFunc);
      faceDesc.StencilDepthFailOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilDepthFailOp);
      faceDesc.StencilFailOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilFailOp);
      faceDesc.StencilPassOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilPassOp);
    }

    // BackFace
    {
      D3D12_DEPTH_STENCILOP_DESC& faceDesc = dsState.BackFace;
      const DepthStencilFaceProperties& faceProps = dsProps.myBackFace;
      faceDesc.StencilFunc = RenderCore_PlatformDX12::ResolveCompFunc(faceProps.myStencilCompFunc);
      faceDesc.StencilDepthFailOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilDepthFailOp);
      faceDesc.StencilFailOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilFailOp);
      faceDesc.StencilPassOp = RenderCore_PlatformDX12::ResolveStencilOp(faceProps.myStencilPassOp);
    }

    // INPUT LAYOUT
    const ShaderDX12* vertexShader = static_cast<const ShaderDX12*>(aState.myShaderPipeline->GetShader(ShaderStage::SHADERSTAGE_VERTEX));
    const VertexInputLayout* inputLayout = aState.myVertexInputLayout ? aState.myVertexInputLayout : vertexShader->myDefaultVertexInputLayout.get();
    ASSERT(inputLayout);
    const VertexInputLayoutProperties& inputLayoutProps = inputLayout->myProperties;

    const eastl::fixed_vector<VertexShaderAttributeDesc, 16>& shaderAttributes = vertexShader->myVertexAttributes;

    eastl::fixed_vector<D3D12_INPUT_ELEMENT_DESC, 16> inputElementDescs;
    const eastl::fixed_vector<VertexInputAttributeDesc, 8>& inputAttributes = inputLayoutProps.myAttributes;
    for (uint i = 0u; i < shaderAttributes.size(); ++i)
    {
      const VertexShaderAttributeDesc& shaderAttribute = shaderAttributes[i];

      int inputAttributeIndex = -1;
      for (uint k = 0u; k < inputAttributes.size(); ++k)
      {
        const VertexInputAttributeDesc& input = inputAttributes[k];
        if (shaderAttribute.mySemantic == input.mySemantic && shaderAttribute.mySemanticIndex == input.mySemanticIndex)
        {
          inputAttributeIndex = (int)k;
          break;
        }
      }

      if (inputAttributeIndex != -1)
      {
        const VertexInputAttributeDesc& input = inputAttributes[inputAttributeIndex];
        
        D3D12_INPUT_ELEMENT_DESC& elementDesc = inputElementDescs.push_back();
        elementDesc.Format = RenderCore_PlatformDX12::ResolveFormat(input.myFormat);
        elementDesc.AlignedByteOffset = inputLayout->myAttributeOffsetsInBuffer[inputAttributeIndex];
        elementDesc.SemanticName = Priv_PipelineStateCacheDX12::VertexAttributeSemanticToString(input.mySemantic);
        elementDesc.SemanticIndex = input.mySemanticIndex;
        ASSERT(input.myBufferIndex < inputLayoutProps.myBufferBindings.size());
        elementDesc.InputSlot = input.myBufferIndex;

        const VertexBufferBindDesc& bufferBindDesc = inputLayoutProps.myBufferBindings[input.myBufferIndex];
        elementDesc.InputSlotClass = bufferBindDesc.myInputRate == VertexInputRate::PER_INSTANCE ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = bufferBindDesc.myInputRate == VertexInputRate::PER_INSTANCE ? 1 : 0;
      }
    }

    D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc = psoDesc.InputLayout;
    inputLayoutDesc.NumElements = static_cast<uint>(inputElementDescs.size());
    inputLayoutDesc.pInputElementDescs = inputElementDescs.data();

    // IB STRIP CUT VALUE
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // TOPOLOGY TYPE
    psoDesc.PrimitiveTopologyType = Adapter::ResolveTopologyType(aState.myTopologyType);

    // SHADERSTAGE_NUM RENDER TARGETS
    psoDesc.NumRenderTargets = aState.myNumRenderTargets;

    // RTV-FORMATS
    for (uint i = 0u; i < aState.myNumRenderTargets; ++i)
    {
      psoDesc.RTVFormats[i] = RenderCore_PlatformDX12::ResolveFormat(aState.myRTVformats[i]);
    }

    // DSV FORMAT
    psoDesc.DSVFormat = RenderCore_PlatformDX12::GetDepthStencilViewFormat(RenderCore_PlatformDX12::ResolveFormat(aState.myDSVformat));

    // NODE MASK
    psoDesc.NodeMask = 0u;

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myGraphicsPsoCache.find(hash);
    if (it != myGraphicsPsoCache.end())
      return it->second;

    ID3D12PipelineState* pso = nullptr;
    ASSERT_HRESULT(RenderCore::GetPlatformDX12()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
    myGraphicsPsoCache[hash] = pso;

    return pso;
  }
//---------------------------------------------------------------------------//
  ID3D12PipelineState* PipelineStateCacheDX12::GetCreateComputePSO(const ComputePipelineState& aState)
  {
    ASSERT(aState.myShaderPipeline != nullptr);
    const ShaderPipelineDX12* shaderPipeline = static_cast<const ShaderPipelineDX12*>(aState.myShaderPipeline);
    ASSERT(shaderPipeline->IsComputePipeline());
    const ShaderDX12* computeShader = static_cast<const ShaderDX12*>(shaderPipeline->GetShader(ShaderStage::SHADERSTAGE_COMPUTE));

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    memset(&desc, 0u, sizeof(desc));

    desc.pRootSignature = RenderCore::GetPlatformDX12()->GetRootSignature()->GetRootSignature();
    desc.CS = computeShader->getNativeByteCode();
    desc.NodeMask = 0u;

    const uint64 hash = aState.GetHash();

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myComputePsoCache.find(hash);
    if (it != myComputePsoCache.end())
      return it->second;

    ID3D12PipelineState* pso = nullptr;
    ASSERT_HRESULT(RenderCore::GetPlatformDX12()->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso)));
    myComputePsoCache[hash] = pso;

    return pso;
  }
//---------------------------------------------------------------------------//
  void PipelineStateCacheDX12::Clear()
  {
    std::lock_guard<std::mutex> lock(myCacheMutex);
    for (auto it : myGraphicsPsoCache)
      it.second->Release();

    myGraphicsPsoCache.clear();

    for (auto it : myComputePsoCache)
      it.second->Release();

    myComputePsoCache.clear();
  }
//---------------------------------------------------------------------------//
}

#endif