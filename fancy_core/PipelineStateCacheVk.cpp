#include "fancy_core_precompile.h"
#include "PipelineStateCacheVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "CommandList.h"
#include "ShaderPipelineVk.h"
#include "ShaderVk.h"
#include "BlendState.h"
#include "DepthStencilState.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  PipelineStateCacheVk::~PipelineStateCacheVk()
  {
    PipelineStateCacheVk::Clear();
  }
//---------------------------------------------------------------------------//
  VkPipeline PipelineStateCacheVk::GetCreateGraphicsPipeline(const GraphicsPipelineState& aState, VkRenderPass aRenderPass)
  {
    const uint64 hash = aState.GetHash();

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myCache.find(hash);
      if (it != myCache.end())
        return it->second;
    }

    ASSERT(aState.myNumRenderTargets <= RenderConstants::kMaxNumRenderTargets);
    ASSERT(aState.myNumRenderTargets > 0 || aState.myDSVformat != DataFormat::NONE);
    ASSERT(aState.myShaderPipeline != nullptr);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.renderPass = aRenderPass;
    pipelineCreateInfo.flags = 0u;

    // Dynamic state (parts of the pipeline state that can be changed on the command-list)
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.pNext = nullptr;
    dynamicStateInfo.flags = 0u;

    // Make those states dynamic that are also dynamic on DX12 to unify the high-level command list API
    VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_DEPTH_BOUNDS,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
    };
    dynamicStateInfo.pDynamicStates = dynamicStates;
    dynamicStateInfo.dynamicStateCount = ARRAY_LENGTH(dynamicStates);
    pipelineCreateInfo.pDynamicState = &dynamicStateInfo;

    // Shader state
    uint numShaderStages = 0;
    VkPipelineShaderStageCreateInfo pipeShaderCreateInfos[(uint)ShaderStage::NUM_NO_COMPUTE] = {};
    for (uint i = 0u; i < (uint)ShaderStage::NUM_NO_COMPUTE; ++i)
    {
      const Shader* shader = aState.myShaderPipeline->GetShader(i);
      if (shader != nullptr)
      {
        const ShaderVk* shaderVk = static_cast<const ShaderVk*>(shader);
        pipeShaderCreateInfos[numShaderStages++] = shaderVk->myShaderStageCreateInfo;
      }
    }
    pipelineCreateInfo.pStages = pipeShaderCreateInfos;
    pipelineCreateInfo.stageCount = numShaderStages;

    // Pipeline layout
    const ShaderPipelineVk* shaderPipelineVk = static_cast<const ShaderPipelineVk*>(aState.myShaderPipeline);
    pipelineCreateInfo.layout = shaderPipelineVk->GetPipelineLayout();

    // Vertex input state
    const ShaderVk* vertexShader = static_cast<const ShaderVk*>(aState.myShaderPipeline->GetShader(ShaderStage::VERTEX));
    const VertexInputLayout* inputLayout = aState.myVertexInputLayout ? aState.myVertexInputLayout : vertexShader->myDefaultVertexInputLayout.get();
    ASSERT(inputLayout);

    StaticArray<VkVertexInputBindingDescription, 16> bindingDescs;
    StaticArray<VkVertexInputAttributeDescription, 16> vkAttributeDescs;

    const VertexInputLayoutProperties& inputLayoutProps = inputLayout->myProperties;
    for (uint i = 0u; i < inputLayoutProps.myBufferBindings.Size(); ++i)
    {
      VkVertexInputBindingDescription& bindingDesc = bindingDescs.Add();
      bindingDesc.binding = i;
      bindingDesc.inputRate = RenderCore_PlatformVk::ResolveVertexInputRate(inputLayoutProps.myBufferBindings[i].myInputRate);
      bindingDesc.stride = inputLayoutProps.myBufferBindings[i].myStride;
    }

    const StaticArray<VertexShaderAttributeDesc, 16>& shaderAttributes = vertexShader->myVertexAttributes;
    const StaticArray<uint, 16>& shaderAttributeLocations = vertexShader->myVertexAttributeLocations;
    const StaticArray<VertexInputAttributeDesc, 16>& inputAttributes = inputLayoutProps.myAttributes;
    for (uint i = 0u; i < shaderAttributes.Size(); ++i)
    {
      const VertexShaderAttributeDesc& shaderAttribute = shaderAttributes[i];
      const uint shaderAttributeLocation = shaderAttributeLocations[i];
      int inputAttributeIndex = -1;
      for (uint k = 0u; k < inputAttributes.Size(); ++k)
      {
        const VertexInputAttributeDesc& input = inputAttributes[k];
        if (shaderAttribute.mySemantic == input.mySemantic && shaderAttribute.mySemanticIndex == input.mySemanticIndex)
        {
          inputAttributeIndex = (int) k;
          break;
        }
      }

      if (inputAttributeIndex != -1)
      {
        const VertexInputAttributeDesc& input = inputAttributes[inputAttributeIndex];
        VkVertexInputAttributeDescription& attributeDesc = vkAttributeDescs.Add();
        ASSERT(input.myBufferIndex < bindingDescs.Size());
        attributeDesc.binding = input.myBufferIndex;
        attributeDesc.format = RenderCore_PlatformVk::ResolveFormat(input.myFormat);
        attributeDesc.location = shaderAttributeLocation;
        attributeDesc.offset = inputLayout->myAttributeOffsetsInBuffer[inputAttributeIndex];
      }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.pNext = nullptr;
    vertexInputCreateInfo.flags = 0u;
    vertexInputCreateInfo.pVertexAttributeDescriptions = vkAttributeDescs.GetBuffer();
    vertexInputCreateInfo.vertexAttributeDescriptionCount = vkAttributeDescs.Size();
    vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescs.GetBuffer();
    vertexInputCreateInfo.vertexBindingDescriptionCount = bindingDescs.Size();

    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.pNext = nullptr;
    inputAssemblyCreateInfo.flags = 0u;
    inputAssemblyCreateInfo.topology = RenderCore_PlatformVk::ResolveTopologyType(aState.myTopologyType);
    inputAssemblyCreateInfo.primitiveRestartEnable = false;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;

    // Multisample state
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.pNext = nullptr;
    multisampleInfo.flags = 0u;
    multisampleInfo.alphaToCoverageEnable = false;
    multisampleInfo.alphaToOneEnable = false;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = false;
    multisampleInfo.minSampleShading = 0.0f;
    VkSampleMask sampleMask = UINT_MAX;
    multisampleInfo.pSampleMask = &sampleMask;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;

    // Color blend state
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.pNext = nullptr;
    colorBlendInfo.flags = 0u;

    const BlendStateProperties& blendProps = aState.myBlendState->GetProperties();
    colorBlendInfo.logicOpEnable = blendProps.myLogicOpEnabled;
    colorBlendInfo.logicOp = RenderCore_PlatformVk::ResolveLogicOp(blendProps.myLogicOp);

    VkPipelineColorBlendAttachmentState colorAttachmentBlendState[RenderConstants::kMaxNumRenderTargets] = {};
    if (aState.myNumRenderTargets > 0)
    {
      auto GetBlendStateForRt = [&](int i)
      {
        const BlendStateRenderTargetProperties& rtBlendProps = blendProps.myRendertargetProperties[i];

        VkPipelineColorBlendAttachmentState rt;
        rt.blendEnable = rtBlendProps.myBlendEnabled;

        const bool alphaSeparateBlend = rtBlendProps.myAlphaSeparateBlend;
        rt.colorBlendOp = RenderCore_PlatformVk::ResolveBlendOp(rtBlendProps.myBlendOp);
        rt.alphaBlendOp = alphaSeparateBlend ? RenderCore_PlatformVk::ResolveBlendOp(rtBlendProps.myBlendOpAlpha) : rt.colorBlendOp;

        rt.srcColorBlendFactor = RenderCore_PlatformVk::ResolveBlendFactor(rtBlendProps.mySrcBlendFactor);
        rt.srcAlphaBlendFactor = alphaSeparateBlend ? RenderCore_PlatformVk::ResolveBlendFactor(rtBlendProps.mySrcBlendAlphaFactor) : rt.srcColorBlendFactor;

        rt.dstColorBlendFactor = RenderCore_PlatformVk::ResolveBlendFactor(rtBlendProps.myDstBlendFactor);
        rt.dstAlphaBlendFactor = alphaSeparateBlend ? RenderCore_PlatformVk::ResolveBlendFactor(rtBlendProps.myDstBlendAlphaFactor) : rt.dstColorBlendFactor;

        const uint writeMask = rtBlendProps.myColorChannelWriteMask;
        const bool red = (writeMask & 0xFF000000) > 0u;
        const bool green = (writeMask & 0x00FF0000) > 0u;
        const bool blue = (writeMask & 0x0000FF00) > 0u;
        const bool alpha = (writeMask & 0x000000FF) > 0u;
        rt.colorWriteMask =
          (red ? VK_COLOR_COMPONENT_R_BIT : 0u)
          | (green ? VK_COLOR_COMPONENT_G_BIT : 0u)
          | (blue ? VK_COLOR_COMPONENT_B_BIT : 0u)
          | (alpha ? VK_COLOR_COMPONENT_A_BIT : 0u);

        return rt;
      };

      colorAttachmentBlendState[0] = GetBlendStateForRt(0);
      if (blendProps.myBlendStatePerRT)
      {
        for (uint i = 1u; i < aState.myNumRenderTargets; ++i)
          colorAttachmentBlendState[i] = GetBlendStateForRt(i);
      }
      else
      {
        for (uint i = 1u; i < aState.myNumRenderTargets; ++i)
          colorAttachmentBlendState[i] = colorAttachmentBlendState[0];
      }
    }
    colorBlendInfo.pAttachments = colorAttachmentBlendState;
    colorBlendInfo.attachmentCount = aState.myNumRenderTargets;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;

    // Depth-stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    const DepthStencilStateProperties& dsProps = aState.myDepthStencilState->GetProperties();
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.flags = 0u;
    depthStencilInfo.depthBoundsTestEnable = false; // TODO: Add support for depthbounds test
    depthStencilInfo.minDepthBounds = 0.0f;  // Will be set as a dynamic state
    depthStencilInfo.maxDepthBounds = 9999.0f;
    depthStencilInfo.depthTestEnable = dsProps.myDepthTestEnabled;
    depthStencilInfo.depthWriteEnable = dsProps.myDepthWriteEnabled;
    depthStencilInfo.stencilTestEnable = dsProps.myStencilEnabled;
    depthStencilInfo.depthCompareOp = RenderCore_PlatformVk::ResolveCompFunc(dsProps.myDepthCompFunc);

    VkStencilOpState* stencilFaceStates[] = { &depthStencilInfo.front, &depthStencilInfo.back };
    const DepthStencilFaceProperties* stencilFaceProps[] = { &dsProps.myFrontFace, &dsProps.myBackFace };

    for (uint i = 0u; i < ARRAY_LENGTH(stencilFaceStates); ++i)
    {
      VkStencilOpState& stencilFaceState = *stencilFaceStates[i];
      const DepthStencilFaceProperties& stencilFaceProp = *stencilFaceProps[i];
      stencilFaceState.writeMask = dsProps.myStencilWriteMask;
      stencilFaceState.compareMask = dsProps.myStencilReadMask;
      stencilFaceState.reference = 0;  // Will be set as a dynamic state
      stencilFaceState.compareOp = RenderCore_PlatformVk::ResolveCompFunc(stencilFaceProp.myStencilCompFunc);
      stencilFaceState.depthFailOp = RenderCore_PlatformVk::ResolveStencilOp(stencilFaceProp.myStencilDepthFailOp);
      stencilFaceState.failOp = RenderCore_PlatformVk::ResolveStencilOp(stencilFaceProp.myStencilFailOp);
      stencilFaceState.passOp = RenderCore_PlatformVk::ResolveStencilOp(stencilFaceProp.myStencilPassOp);
    }

    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterInfo = {};
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.pNext = nullptr;
    rasterInfo.flags = 0u;
    rasterInfo.cullMode = RenderCore_PlatformVk::ResolveCullMode(aState.myCullMode);
    rasterInfo.frontFace = RenderCore_PlatformVk::ResolveWindingOrder(aState.myWindingOrder);
    rasterInfo.polygonMode = RenderCore_PlatformVk::ResolveFillMode(aState.myFillMode);
    // TODO: Add support for these features below...
    rasterInfo.depthBiasEnable = false;
    rasterInfo.depthClampEnable = false;
    rasterInfo.rasterizerDiscardEnable = false;
    rasterInfo.depthBiasClamp = 0.0f;
    rasterInfo.depthBiasConstantFactor = 1.0f;
    rasterInfo.depthBiasSlopeFactor = 1.0f;
    rasterInfo.lineWidth = 1.0f;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;

    // Tesselation state
    VkPipelineTessellationStateCreateInfo tesselationInfo = {};
    tesselationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tesselationInfo.pNext = nullptr;
    tesselationInfo.flags = 0u;
    tesselationInfo.patchControlPoints = 1u;
    pipelineCreateInfo.pTessellationState = &tesselationInfo;

    // Viewport state (will be handled as a dynamic state on the command list)
    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pNext = nullptr;
    viewportInfo.flags = 0u;

    viewportInfo.pViewports = nullptr;  // Part of the dynamic states
    viewportInfo.viewportCount = 1u;  // Must be 1, even though the pointer is nullptr
    viewportInfo.pScissors = nullptr;
    viewportInfo.scissorCount = 1u;

    pipelineCreateInfo.pViewportState = &viewportInfo;

    pipelineCreateInfo.renderPass = aRenderPass;
    pipelineCreateInfo.subpass = 0u;

    VkPipeline pipeline;
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(hash);
    if (it != myCache.end())
      return it->second;

    ASSERT_VK_RESULT(vkCreateGraphicsPipelines(platformVk->myDevice, nullptr, 1u, &pipelineCreateInfo, nullptr, &pipeline));
    myCache[hash] = pipeline;
    return pipeline;
  }
//---------------------------------------------------------------------------//
  VkPipeline PipelineStateCacheVk::GetCreateComputePipeline(const ComputePipelineState& aState)
  {
    ASSERT(aState.myShaderPipeline != nullptr);
    const ShaderPipelineVk* shaderPipeline = static_cast<const ShaderPipelineVk*>(aState.myShaderPipeline);
    ASSERT(shaderPipeline->IsComputePipeline());
    const ShaderVk* computeShader = static_cast<const ShaderVk*>(shaderPipeline->GetShader(ShaderStage::COMPUTE));

    VkComputePipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_PIPELINE_CREATE_DISPATCH_BASE;
    createInfo.stage = computeShader->GetStageCreateInfo();
    createInfo.layout = shaderPipeline->GetPipelineLayout();

    const uint64 hash = aState.GetHash();

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(hash);
    if (it != myCache.end())
      return it->second;

    VkPipeline pipeline = nullptr;
    ASSERT_VK_RESULT(vkCreateComputePipelines(RenderCore::GetPlatformVk()->myDevice, nullptr, 1u, &createInfo, nullptr, &pipeline));

    return pipeline;
  }
//---------------------------------------------------------------------------//
  void PipelineStateCacheVk::Clear()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    std::lock_guard<std::mutex> lock(myCacheMutex);

    for (auto it : myCache)
      vkDestroyPipeline(RenderCore::GetPlatformVk()->GetDevice(), it.second, nullptr);

    myCache.clear();
  }
//---------------------------------------------------------------------------//
}
