#include "fancy_core_precompile.h"
#include "CommandListVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderPipeline.h"
#include "ShaderVk.h"
#include "ShaderPipelineVk.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "Texture.h"
#include "TextureVk.h"
#include "GpuResourceViewDataVk.h"
#include "GpuBufferVk.h"
#include "ShaderResourceInfoVk.h"
#include "TextureSamplerVk.h"
#include "DynamicArray.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Priv_CommandListVk
  {
    VkRenderPass locCreateRenderPass(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget)
    {
      VkRenderPassCreateInfo renderpassInfo;
      renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderpassInfo.pNext = nullptr;
      renderpassInfo.flags = 0u;

      VkAttachmentDescription attachmentDescriptions[RenderConstants::kMaxNumRenderTargets + 1u];
      VkAttachmentReference colorAttachmentRefs[RenderConstants::kMaxNumRenderTargets];
      VkAttachmentReference depthStencilAttachmentRef;
      for (uint i = 0u; i < aNumRenderTargets; ++i)
      {
        VkAttachmentDescription& attachmentDesc = attachmentDescriptions[i];
        attachmentDesc.flags = 0u;
        attachmentDesc.format = RenderCore_PlatformVk::ResolveFormat(someRendertargets[i]->GetProperties().myFormat);
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        // Image layouts: The high-level code is responsible for transitioning the rendertargets
        // into VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL before being used as a rendertarget here.
        // The renderpass itself does not modify the layout and it is up to higher-level code again to transition out of
        // the color-attachment-optimal layout.
        const VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDesc.initialLayout = layout;
        attachmentDesc.finalLayout = layout;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkAttachmentReference& attachmentRef = colorAttachmentRefs[i];
        attachmentRef.attachment = i;
        attachmentRef.layout = layout;
      }

      const bool hasDepthStencilTarget = aDepthStencilTarget != nullptr;
      if (hasDepthStencilTarget)
      {
        const TextureViewProperties& dsvProps = aDepthStencilTarget->GetProperties();

        VkAttachmentDescription& attachmentDesc = attachmentDescriptions[aNumRenderTargets];
        attachmentDesc.flags = 0u;
        attachmentDesc.format = RenderCore_PlatformVk::ResolveFormat(dsvProps.myFormat);
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

        VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (dsvProps.myIsDepthReadOnly && dsvProps.myIsStencilReadOnly)
          layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        else if (dsvProps.myIsDepthReadOnly)
          layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        else if (dsvProps.myIsStencilReadOnly)
          layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

        attachmentDesc.initialLayout = layout;
        attachmentDesc.finalLayout = layout;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

        depthStencilAttachmentRef.attachment = aNumRenderTargets;
        depthStencilAttachmentRef.layout = layout;
      }
      renderpassInfo.attachmentCount = aNumRenderTargets + (aDepthStencilTarget != nullptr ? 1u : 0u);
      renderpassInfo.pAttachments = attachmentDescriptions;

      VkSubpassDescription subpassDesc;
      subpassDesc.flags = 0u;
      subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpassDesc.colorAttachmentCount = aNumRenderTargets;
      subpassDesc.pColorAttachments = colorAttachmentRefs;
      subpassDesc.pDepthStencilAttachment = aDepthStencilTarget != nullptr ? &depthStencilAttachmentRef : nullptr;
      subpassDesc.pResolveAttachments = nullptr;

      // Input attachments are a way to read the current value of a pixel that's currently being written in a fragment shader.
      // This would enable some advanced effects like transparency and things like that without having to use multiple passes.
      // For now, we'll disable that here, but will look into supporting this feature in the future if it has a corresponding feature in DX12
      subpassDesc.inputAttachmentCount = 0u;
      subpassDesc.pInputAttachments = nullptr;

      subpassDesc.preserveAttachmentCount = 0u;
      subpassDesc.pPreserveAttachments = nullptr;

      renderpassInfo.pSubpasses = &subpassDesc;
      renderpassInfo.subpassCount = 1u;
      renderpassInfo.pDependencies = nullptr;
      renderpassInfo.dependencyCount = 0u;

      VkRenderPass renderPass;
      ASSERT_VK_RESULT(vkCreateRenderPass(RenderCore::GetPlatformVk()->myDevice, &renderpassInfo, nullptr, &renderPass));

      return renderPass;
    }
//---------------------------------------------------------------------------//      
    VkFramebuffer locCreateFramebuffer(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget, glm::uvec2 aFramebufferRes, VkRenderPass aRenderPass)
    {
      VkImageView attachments[RenderConstants::kMaxNumRenderTargets + 1u];
      for (uint i = 0u; i < aNumRenderTargets; ++i)
      {
        const GpuResourceViewDataVk& viewDataVk = someRendertargets[i]->myNativeData.To<GpuResourceViewDataVk>();
        attachments[i] = viewDataVk.myView.myImage;
      }

      const bool hasDepthStencilTarget = aDepthStencilTarget != nullptr;
      if (hasDepthStencilTarget)
      {
        const GpuResourceViewDataVk& viewDataVk = someRendertargets[aNumRenderTargets]->myNativeData.To<GpuResourceViewDataVk>();
        attachments[aNumRenderTargets] = viewDataVk.myView.myImage;
      }

      VkFramebufferCreateInfo framebufferInfo;
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.pNext = nullptr;
      framebufferInfo.flags = 0u;
      framebufferInfo.renderPass = aRenderPass;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.attachmentCount = hasDepthStencilTarget ? aNumRenderTargets + 1u : aNumRenderTargets;
      framebufferInfo.width = aFramebufferRes.x;
      framebufferInfo.height = aFramebufferRes.y;
      framebufferInfo.layers = 1u;  // TODO: Support layered rendering

      VkFramebuffer framebuffer = nullptr;
      ASSERT_VK_RESULT(vkCreateFramebuffer(RenderCore::GetPlatformVk()->myDevice, &framebufferInfo, nullptr, &framebuffer));
      return framebuffer;
    }
//---------------------------------------------------------------------------//
    VkPipeline locCreateGraphicsPipeline(const GraphicsPipelineState& aState, VkRenderPass aRenderPass)
    {
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
      for (const SharedPtr<Shader>& shader : aState.myShaderPipeline->myShaders)
      {
        if (shader != nullptr)
        {
          const ShaderVk* shaderVk = static_cast<const ShaderVk*>(shader.get());
          pipeShaderCreateInfos[numShaderStages++] = shaderVk->myShaderStageCreateInfo;
        }
      }
      pipelineCreateInfo.pStages = pipeShaderCreateInfos;
      pipelineCreateInfo.stageCount = numShaderStages;

      // Pipeline layout
      const ShaderPipelineVk* shaderPipelineVk = static_cast<const ShaderPipelineVk*>(aState.myShaderPipeline.get());
      pipelineCreateInfo.layout = shaderPipelineVk->myPipelineLayout;

      // Vertex input state
      const ShaderVk* vertexShader = static_cast<const ShaderVk*>(aState.myShaderPipeline->myShaders[(uint)ShaderStage::VERTEX].get());

      VkVertexInputBindingDescription vertexBindingDesc = {};
      vertexBindingDesc.binding = 0;
      vertexBindingDesc.stride = vertexShader->myVertexAttributeDesc.myOverallVertexSize;
      vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      // TODO: Rework this part so that the user can define how the vertex-binding is to be set up.
      VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
      vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputCreateInfo.pNext = nullptr;
      vertexInputCreateInfo.flags = 0u;
      vertexInputCreateInfo.pVertexAttributeDescriptions = vertexShader->myVertexAttributeDesc.myVertexAttributes.data();
      vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint)vertexShader->myVertexAttributeDesc.myVertexAttributes.size();
      vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDesc;
      vertexInputCreateInfo.vertexBindingDescriptionCount = 1u;

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
      VkPipelineViewportStateCreateInfo viewportInfo ={};
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
      ASSERT_VK_RESULT(vkCreateGraphicsPipelines(platformVk->myDevice, nullptr, 1u, &pipelineCreateInfo, nullptr, &pipeline));
      return pipeline;
    }
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskGraphics = static_cast<D3D12_RESOURCE_STATES>(~0u);
    constexpr VkAccessFlags locAccessMaskCompute = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT
      | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT
      | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
      VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX | VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT | VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskWrite = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT |
      VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT | VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
//---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locPipelineMaskGraphics = static_cast<D3D12_RESOURCE_STATES>(~0u);
  //---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locPipelineMaskCompute = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locGetContextAccessMask(CommandListType aCommandListType)
    {
      switch (aCommandListType) 
      { 
        case CommandListType::Graphics: return locAccessMaskGraphics;
        case CommandListType::Compute: return locAccessMaskCompute;
        default: ASSERT(false, "Missing implementation"); return 0u;
      }
    }
//---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locGetContextPipelineStageMask(CommandListType aCommandListType)
    {
      switch (aCommandListType)
      {
      case CommandListType::Graphics: return locPipelineMaskGraphics;
      case CommandListType::Compute: return locPipelineMaskCompute;
      default: ASSERT(false, "Missing implementation"); return 0u;
      }
    }
//---------------------------------------------------------------------------//
    VkAccessFlags locResolveValidateDstAccessMask(const GpuResource* aResource, CommandListType aCommandListType, VkAccessFlags aAccessFlags)
    {
      const VkAccessFlags contextAccessMask = Priv_CommandListVk::locGetContextAccessMask(aCommandListType);
      VkAccessFlags dstFlags = aAccessFlags & contextAccessMask;
      ASSERT(dstFlags != 0, "Unsupported access mask for this commandlist type");
      ASSERT((dstFlags & Priv_CommandListVk::locAccessMaskRead) == dstFlags || (dstFlags & Priv_CommandListVk::locAccessMaskWrite) == dstFlags, "Simulataneous read- and write access flags are not allowed");

      const bool dstIsRead = (dstFlags & Priv_CommandListVk::locAccessMaskRead) == dstFlags;
      dstFlags = dstFlags & (dstIsRead ? aResource->GetHazardData().myVkData.myReadAccessMask : aResource->GetHazardData().myVkData.myWriteAccessMask);
      ASSERT(dstFlags != 0, "Dst access flags not supported by resource");

      return dstFlags;
    }
//---------------------------------------------------------------------------//
    bool locValidateDstImageLayout(const GpuResource* aResource, VkImageLayout anImageLayout)
    {
      if (aResource->myCategory == GpuResourceCategory::BUFFER)
      {
        ASSERT(anImageLayout == VK_IMAGE_LAYOUT_UNDEFINED);
        return anImageLayout == VK_IMAGE_LAYOUT_UNDEFINED;
      }
      else
      {
        DynamicArray<uint>& supportedLayouts = aResource->GetHazardData().myVkData.mySupportedImageLayouts;
        const bool supportsLayout = std::find(supportedLayouts.begin(), supportedLayouts.end(), (uint) anImageLayout) != supportedLayouts.end();
        ASSERT(supportsLayout);
        return supportsLayout;
      }
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  std::unordered_map<uint64, VkPipeline> CommandListVk::ourPipelineCache;
  std::unordered_map<uint64, VkRenderPass> CommandListVk::ourRenderpassCache;
  std::unordered_map<uint64, VkFramebuffer> CommandListVk::ourFramebufferCache;
//---------------------------------------------------------------------------//
  CommandListVk::CommandListVk(CommandListType aType) 
    : CommandList(aType)
    , myCommandBuffer(nullptr)
    , myRenderPass(nullptr)
    , myFramebuffer(nullptr)
    , myFramebufferRes(0u, 0u)
    , myPendingBarrierSrcStageMask(0u)
    , myPendingBarrierDstStageMask(0u)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    myCommandBuffer = platformVk->GetNewCommandBuffer(myCommandListType);

    BeginCommandBuffer();
  }
//---------------------------------------------------------------------------//
  CommandListVk::~CommandListVk()
  {
    CommandListVk::PostExecute(0ull);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    VkClearColorValue clearColor;
    memcpy(clearColor.float32, aColor, sizeof(clearColor.float32));

    VkImageSubresourceRange subRange = RenderCore_PlatformVk::ResolveSubresourceRange(aTextureView->mySubresourceRange, 
      aTextureView->GetTexture()->GetProperties().myFormat);

    TrackSubresourceTransition(aTextureView->GetResource(), aTextureView->GetSubresourceRange(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();

    GpuResourceDataVk* dataVk = static_cast<TextureVk*>(aTextureView->GetTexture())->GetData();
    vkCmdClearColorImage(myCommandBuffer, dataVk->myImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1u, &subRange);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const
                                          TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureToBufferCopy(aDstBuffer->GetProperties(), aDstOffset, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif  // FANCY_RENDERER_USE_VALIDATION

    VkBufferImageCopy copyRegion;
    copyRegion.bufferOffset = aDstOffset;
    // Using 0 here will make it fall back to imageExtent. 
    // The buffer is expected to have the copied texture region tightly packed in memory
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.baseArrayLayer = aSrcSubresource.myArrayIndex;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageSubresource.aspectMask =
      RenderCore_PlatformVk::ResolveAspectMask(aSrcSubresource.myPlaneIndex, 1u, aSrcTexture->GetProperties().myFormat);
    copyRegion.imageSubresource.mipLevel = aSrcSubresource.myMipLevel;
    copyRegion.imageOffset.x = aSrcRegion.myPos.x;
    copyRegion.imageOffset.y = aSrcRegion.myPos.y;
    copyRegion.imageOffset.z = aSrcRegion.myPos.z;
    copyRegion.imageExtent.width = aSrcRegion.mySize.x;
    copyRegion.imageExtent.height = aSrcRegion.mySize.y;
    copyRegion.imageExtent.depth = aSrcRegion.mySize.z;

    VkBuffer dstBuffer = static_cast<const GpuBufferVk*>(aDstBuffer)->GetData()->myBuffer;
    VkImage srcImage = static_cast<const TextureVk*>(aSrcTexture)->GetData()->myImage;

    const VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackResourceTransition(aDstBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();
    
    vkCmdCopyImageToBuffer(myCommandBuffer, srcImage, srcImageLayout, dstBuffer, 1u, &copyRegion);   
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource,
                                  const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const
                                  TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif

    VkImageCopy copyRegion;
    copyRegion.extent.width = aDstRegion.mySize.x;
    copyRegion.extent.height = aDstRegion.mySize.y;
    copyRegion.extent.depth = aDstRegion.mySize.z;
    copyRegion.dstOffset.x = aDstRegion.myPos.x;
    copyRegion.dstOffset.y = aDstRegion.myPos.y;
    copyRegion.dstOffset.z = aDstRegion.myPos.z;
    copyRegion.srcOffset.x = aSrcRegion.myPos.x;
    copyRegion.srcOffset.y = aSrcRegion.myPos.y;
    copyRegion.srcOffset.z = aSrcRegion.myPos.z;
    copyRegion.srcSubresource.mipLevel = aSrcSubresource.myMipLevel;
    copyRegion.srcSubresource.baseArrayLayer = aSrcSubresource.myArrayIndex;
    copyRegion.srcSubresource.layerCount = 1u;
    copyRegion.srcSubresource.aspectMask = RenderCore_PlatformVk::ResolveAspectMask(aSrcSubresource.myPlaneIndex, 1u, aSrcTexture->GetProperties().myFormat);
    copyRegion.dstSubresource.mipLevel = aDstSubresource.myMipLevel;
    copyRegion.dstSubresource.baseArrayLayer = aDstSubresource.myArrayIndex;
    copyRegion.dstSubresource.layerCount = 1u;
    copyRegion.dstSubresource.aspectMask = RenderCore_PlatformVk::ResolveAspectMask(aDstSubresource.myPlaneIndex, 1u, aDstTexture->GetProperties().myFormat);

    VkImage dstImage = static_cast<const TextureVk*>(aDstTexture)->GetData()->myImage;
    VkImage srcImage = static_cast<const TextureVk*>(aSrcTexture)->GetData()->myImage;

    const VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    const VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();

    vkCmdCopyImage(myCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1u, &copyRegion);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource,
                                          const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferToTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcBuffer->GetProperties(), aSrcOffset);
#endif

    VkBufferImageCopy copyRegion;
    copyRegion.bufferOffset = aSrcOffset;
    // Using 0 here will make it fall back to imageExtent. 
    // The buffer is expected to have the copied texture region tightly packed in memory
    copyRegion.bufferRowLength = 0u;
    copyRegion.bufferImageHeight = 0u;
    copyRegion.imageSubresource.baseArrayLayer = aDstSubresource.myArrayIndex;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageSubresource.aspectMask = 
      RenderCore_PlatformVk::ResolveAspectMask(aDstSubresource.myPlaneIndex, 1u, aDstTexture->GetProperties().myFormat);
    copyRegion.imageSubresource.mipLevel = aDstSubresource.myMipLevel;
    copyRegion.imageOffset.x = aDstRegion.myPos.x;
    copyRegion.imageOffset.y = aDstRegion.myPos.y;
    copyRegion.imageOffset.z = aDstRegion.myPos.z;
    copyRegion.imageExtent.width = aDstRegion.mySize.x;
    copyRegion.imageExtent.height = aDstRegion.mySize.y;
    copyRegion.imageExtent.depth = aDstRegion.mySize.z;

    VkImage dstImage = static_cast<const TextureVk*>(aDstTexture)->GetData()->myImage;
    VkBuffer srcBuffer = static_cast<const GpuBufferVk*>(aSrcBuffer)->GetData()->myBuffer;

    const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // Texture is expected in the WRITE_COPY_DST state here
    TrackResourceTransition(aSrcBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), VK_ACCESS_TRANSFER_WRITE_BIT, imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();
    
    vkCmdCopyBufferToImage(myCommandBuffer, srcBuffer, dstImage, imageLayout, 1u, &copyRegion);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::PostExecute(uint64 aFenceVal)
  {
    CommandList::PostExecute(aFenceVal);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    
    platformVk->ReleaseCommandBuffer(myCommandBuffer, myCommandListType, aFenceVal);
    myCommandBuffer = nullptr;

    for (uint i = 0u; i < myUsedDescriptorPools.Size(); ++i)
      platformVk->FreeDescriptorPool(myUsedDescriptorPools[i], aFenceVal);
    myUsedDescriptorPools.ClearDiscard();

    for (uint i = 0u; i < myResourceState.myTempBufferViews.Size(); ++i)
      myResourceState.myTempBufferViews[i].second = glm::max(myResourceState.myTempBufferViews[i].second, aFenceVal);

    myLocalHazardData.clear();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::PreBegin()
  {
    CommandList::PreBegin();

    myRenderPass = nullptr;
    myFramebuffer = nullptr;
    myFramebufferRes = glm::uvec2(0u, 0u);
    myPendingBufferBarriers.ClearDiscard();
    myPendingImageBarriers.ClearDiscard();
    myLocalHazardData.clear();

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    myCommandBuffer = platformVk->GetNewCommandBuffer(myCommandListType);
    ASSERT_VK_RESULT(vkResetCommandBuffer(myCommandBuffer, 0u));

    ClearResourceState();

    BeginCommandBuffer();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::FlushBarriers()
  {
    if (myPendingBufferBarriers.IsEmpty() && myPendingImageBarriers.IsEmpty())
      return;

    // ASSERT(myPendingBarrierSrcStageMask != 0u && myPendingBarrierDstStageMask != 0u);

    VkImageMemoryBarrier imageBarriersVk[kNumCachedBarriers];
    VkBufferMemoryBarrier bufferBarriersVk[kNumCachedBarriers];

    for (uint i = 0u, e = myPendingImageBarriers.Size(); i < e; ++i)
    {
      VkImageMemoryBarrier& imageBarrierVk = imageBarriersVk[i];
      const ImageMemoryBarrierData& pendingBarrier = myPendingImageBarriers[i];

      imageBarrierVk.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imageBarrierVk.pNext = nullptr;
      imageBarrierVk.subresourceRange = RenderCore_PlatformVk::ResolveSubresourceRange(pendingBarrier.mySubresourceRange, pendingBarrier.myFormat);
      imageBarrierVk.image = pendingBarrier.myImage;
      imageBarrierVk.srcAccessMask = pendingBarrier.mySrcAccessMask;
      imageBarrierVk.dstAccessMask = pendingBarrier.myDstAccessMask;
      imageBarrierVk.oldLayout = pendingBarrier.mySrcLayout;
      imageBarrierVk.newLayout = pendingBarrier.myDstLayout;
      imageBarrierVk.srcQueueFamilyIndex = pendingBarrier.mySrcQueueFamilyIndex;
      imageBarrierVk.dstQueueFamilyIndex = pendingBarrier.myDstQueueFamilyIndex;
    }

    for (uint i = 0u, e = myPendingBufferBarriers.Size(); i < e; ++i)
    {
      VkBufferMemoryBarrier& bufferBarrierVk = bufferBarriersVk[i];
      const BufferMemoryBarrierData& pendingBarrier = myPendingBufferBarriers[i];

      bufferBarrierVk.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      bufferBarrierVk.pNext = nullptr;
      bufferBarrierVk.buffer = pendingBarrier.myBuffer;
      bufferBarrierVk.srcAccessMask = pendingBarrier.mySrcAccessMask;
      bufferBarrierVk.dstAccessMask = pendingBarrier.myDstAccessMask;
      bufferBarrierVk.srcQueueFamilyIndex = pendingBarrier.mySrcQueueFamilyIndex;
      bufferBarrierVk.dstQueueFamilyIndex = pendingBarrier.myDstQueueFamilyIndex;
    }

    // TODO: Make this more optimial. BOTTOM_OF_PIPE->TOP_OF_PIPE will stall much more than necessary in most cases. But the last writing pipeline stage needs to be tracked per subresource to correctly do this
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    const VkDependencyFlags dependencyFlags = 0u;
    vkCmdPipelineBarrier(myCommandBuffer, srcStageMask, dstStageMask, 
      dependencyFlags, 0u, nullptr, 
      myPendingBufferBarriers.Size(), bufferBarriersVk, 
      myPendingImageBarriers.Size(), imageBarriersVk);

    myPendingBufferBarriers.ClearDiscard();
    myPendingImageBarriers.ClearDiscard();
    
    myPendingBarrierSrcStageMask = 0u;
    myPendingBarrierDstStageMask = 0u;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline)
  {
    if (myGraphicsPipelineState.myShaderPipeline != aShaderPipeline)
      ClearResourceState();

    CommandList::SetShaderPipeline(aShaderPipeline);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset, uint64 /*aSize*/)
  {
    GpuResourceDataVk* resourceDataVk = static_cast<const GpuBufferVk*>(aBuffer)->GetData();
    vkCmdBindVertexBuffers(myCommandBuffer, 0u, 1u, &resourceDataVk->myBuffer, &anOffset);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 /*aSize*/)
  {
    ASSERT(anIndexSize == 2u || anIndexSize == 4u);
    const VkIndexType indexType = anIndexSize == 2u ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

    GpuResourceDataVk* resourceDataVk = static_cast<const GpuBufferVk*>(aBuffer)->GetData();
    vkCmdBindIndexBuffer(myCommandBuffer, resourceDataVk->myBuffer, anOffset, indexType);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance)
  {
    FlushBarriers();
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyGraphicsPipelineState();
    ApplyResourceState();

    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = nullptr;
    renderPassBegin.renderArea.offset = { 0, 0 };
    renderPassBegin.renderArea.extent = { myFramebufferRes.x, myFramebufferRes.y };
    renderPassBegin.clearValueCount = 0u;
    renderPassBegin.pClearValues = nullptr;
    renderPassBegin.renderPass = myRenderPass;
    renderPassBegin.framebuffer = myFramebuffer;

    vkCmdBeginRenderPass(myCommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdDrawIndexed(myCommandBuffer, aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);

    vkCmdEndRenderPass(myCommandBuffer);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas)
  {
    const uint numSubresources = aSubresourceRange.GetNumSubresources();
    ASSERT(aNumDatas == numSubresources);

    const TextureProperties& texProps = aDstTexture->GetProperties();
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    
    uint64* rowSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferRowSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferSliceSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferSubresourceSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint* heights = (uint*)alloca(sizeof(uint) * numSubresources);
    uint* depths = (uint*)alloca(sizeof(uint) * numSubresources);

    uint i = 0;
    uint64 requiredBufferSize = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it)
    {
      const SubresourceLocation subResource = *it;

      uint width, height, depth;
      texProps.GetSize(subResource.myMipLevel, width, height, depth);

      const uint64 rowSize = width * formatInfo.myCopyableSizePerPlane[subResource.myPlaneIndex];
      const uint64 alignedRowSize = MathUtil::Align(rowSize, caps.myTextureRowAlignment);
      const uint64 alignedSliceSize = alignedRowSize * height;
      const uint64 alignedSubresourceSize = MathUtil::Align(alignedSliceSize * depth, caps.myTextureSubresourceBufferAlignment);
      requiredBufferSize += alignedSubresourceSize;
  
      rowSizes[i] = rowSize;
      bufferRowSizes[i] = alignedRowSize;
      bufferSliceSizes[i] = alignedSliceSize;
      bufferSubresourceSizes[i] = alignedSubresourceSize;
      heights[i] = height;
      depths[i] = depth;
      ++i;
    }

    uint64 uploadBufferOffset;
    uint8* uploadBufferData;
    const GpuBuffer* uploadBuffer = GetMappedBuffer(uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, &uploadBufferData, requiredBufferSize);
    ASSERT(uploadBuffer != nullptr);
    ASSERT(uploadBufferData != nullptr);

    uint8* dstSubresourceData = uploadBufferData;
    for (i = 0; i < aNumDatas; ++i)
    {
      const TextureSubData& srcData = someDatas[i];
      ASSERT(rowSizes[i] == srcData.myRowSizeBytes);

      const uint depth = depths[i];
      const uint height = heights[i];

      const uint64 dstRowSize = bufferRowSizes[i];
      const uint64 dstSliceSize = bufferSliceSizes[i];
      const uint64 dstSubresourceSize = bufferSubresourceSizes[i];

      uint8* dstSliceData = dstSubresourceData;
      const uint8* srcSliceData = srcData.myData;
      for (uint d = 0; d < depth; ++d)
      {
        uint8* dstRowData = dstSliceData;
        const uint8* srcRowData = srcSliceData;
        for (uint h = 0; h < height; ++h)
        {
          memcpy(dstRowData, srcRowData, srcData.myRowSizeBytes);

          dstRowData += dstRowSize;
          srcRowData += srcData.myRowSizeBytes;
        }

        dstSliceData += dstSliceSize;
        srcSliceData += srcData.mySliceSizeBytes;
      }

      dstSubresourceData += dstSubresourceSize;
    }

    i = 0;
    uint64 bufferOffset = uploadBufferOffset;
    for (SubresourceIterator subIter = aSubresourceRange.Begin(), e = aSubresourceRange.End(); subIter != e; ++subIter)
    {
      const SubresourceLocation dstLocation = *subIter;
      CommandList::CopyBufferToTexture(aDstTexture, dstLocation, uploadBuffer, bufferOffset);

      bufferOffset += bufferSubresourceSizes[i++];
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindResourceView(const GpuResourceView* aView, uint64 aNameHash, uint anArrayIndex /* = 0u */)
  {
    ShaderResourceInfoVk resourceInfo;
    if (!FindShaderResourceInfo(aNameHash, resourceInfo))
      return;

    const GpuResourceViewDataVk& viewDataVk = aView->myNativeData.To<GpuResourceViewDataVk>();
    const GpuResourceDataVk* resourceDataVk = aView->myResource->myNativeData.To<GpuResourceDataVk*>();

    VkImageView imageView = nullptr;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkBufferView bufferView = nullptr;
    VkBuffer buffer = nullptr;
    uint64 bufferOffset = 0ull;
    uint64 bufferSize = 0ull;
    switch (resourceInfo.myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break;  // Needs to be handled in BindSampler()
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    {
      ASSERT(aView->myType == GpuResourceViewType::SRV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::TEXTURE);
      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageView = viewDataVk.myView.myImage;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::TEXTURE);
      imageLayout = VK_IMAGE_LAYOUT_GENERAL;
      imageView = viewDataVk.myView.myImage;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::BUFFER);
      const GpuBufferViewVk* bufferViewVk = static_cast<const GpuBufferViewVk*>(aView);
      ASSERT(bufferViewVk->GetBufferView() != nullptr);
      bufferView = bufferViewVk->GetBufferView();
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::BUFFER);
      buffer = resourceDataVk->myBuffer;
      bufferOffset = static_cast<const GpuBufferView*>(aView)->GetProperties().myOffset;
      bufferSize = static_cast<const GpuBufferView*>(aView)->GetProperties().mySize;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::SRV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::BUFFER);
      const GpuBufferViewVk* bufferViewVk = static_cast<const GpuBufferViewVk*>(aView);
      ASSERT(bufferViewVk->GetBufferView() != nullptr);
      bufferView = bufferViewVk->GetBufferView();
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::CBV);
      ASSERT(aView->myResource->myCategory == GpuResourceCategory::BUFFER);
      buffer = resourceDataVk->myBuffer;
      bufferOffset = static_cast<const GpuBufferView*>(aView)->GetProperties().myOffset;
      bufferSize = static_cast<const GpuBufferView*>(aView)->GetProperties().mySize;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    default: ASSERT(false);
    }

    BindInternal(resourceInfo, anArrayIndex, bufferView, buffer, bufferOffset, bufferSize, imageView, imageLayout, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash, uint anArrayIndex/* = 0u*/)
  {
    ShaderResourceInfoVk resourceInfo;
    if (!FindShaderResourceInfo(aNameHash, resourceInfo))
      return;

    const GpuBufferProperties& bufferProps = aBuffer->GetProperties();

    bool needsTempBufferView = false;
    switch (resourceInfo.myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break;  // Needs to be handled in BindSampler()
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: ASSERT(false); break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: ASSERT(false); break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {
      ASSERT(someViewProperties.myIsShaderWritable && bufferProps.myIsShaderWritable && (bufferProps.myBindFlags & (uint) GpuBufferBindFlags::SHADER_BUFFER));
      needsTempBufferView = true;
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
      ASSERT(someViewProperties.myIsShaderWritable && bufferProps.myIsShaderWritable && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER));
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    {
      ASSERT(!someViewProperties.myIsShaderWritable && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER));
      needsTempBufferView = true;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      ASSERT(!someViewProperties.myIsShaderWritable && someViewProperties.myIsConstantBuffer && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::CONSTANT_BUFFER));
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    default: ASSERT(false);
    }

    VkBufferView bufferView = nullptr;
    if (needsTempBufferView)
    {
      bufferView = GpuBufferViewVk::CreateVkBufferView(aBuffer, someViewProperties);
      myResourceState.myTempBufferViews.Add({ bufferView, 0u });
    }

    BindInternal(resourceInfo, anArrayIndex, bufferView, static_cast<const GpuBufferVk*>(aBuffer)->GetData()->myBuffer, 
      someViewProperties.myOffset, someViewProperties.mySize, nullptr, VK_IMAGE_LAYOUT_UNDEFINED, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindSampler(const TextureSampler* aSampler, uint64 aNameHash, uint anArrayIndex/* = 0u*/)
  {
    ShaderResourceInfoVk resourceInfo;
    if (!FindShaderResourceInfo(aNameHash, resourceInfo))
      return;
  
    ASSERT(resourceInfo.myType == VK_DESCRIPTOR_TYPE_SAMPLER);

    VkSampler sampler = static_cast<const TextureSamplerVk*>(aSampler)->GetVkSampler();
    BindInternal(resourceInfo, anArrayIndex, nullptr, nullptr, 0ull, 0ull, nullptr, VK_IMAGE_LAYOUT_UNDEFINED, sampler);
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::BeginQuery(GpuQueryType aType)
  {
    VK_MISSING_IMPLEMENTATION();
    return GpuQuery(GpuQueryType::TIMESTAMP, 0u, 0ull, myCommandListType);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::EndQuery(const GpuQuery& aQuery)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::InsertTimestamp()
  {
    VK_MISSING_IMPLEMENTATION();
    return GpuQuery();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Close()
  {
    if (myIsOpen)
      ASSERT_VK_RESULT(vkEndCommandBuffer(myCommandBuffer));

    myIsOpen = false;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::SetComputeProgram(const Shader* aProgram)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Dispatch(const glm::int3& aNumThreads)
  {
    FlushBarriers();

    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TrackResourceTransition(const GpuResource* aResource, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aIsSharedReadState)
  {
    TrackSubresourceTransition(aResource, aResource->GetSubresources(), aNewAccessFlags, aNewImageLayout, aNewPipelineStageFlags, aIsSharedReadState);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TrackSubresourceTransition(
    const GpuResource* aResource, 
    const SubresourceRange& aSubresourceRange, 
    VkAccessFlags aNewAccessFlags, 
    VkImageLayout aNewImageLayout, 
    VkPipelineStageFlags aNewPipelineStageFlags, 
    bool aIsSharedReadState)
  {
    if (aResource->IsBuffer())
    {
      ASSERT(aSubresourceRange.GetNumSubresources() == 1u);
      ASSERT(aNewImageLayout == VK_IMAGE_LAYOUT_UNDEFINED);
    }

    const bool canEarlyOut = !aIsSharedReadState;

    if (!aResource->myStateTracking.myCanChangeStates && canEarlyOut)
      return;

    VkAccessFlags dstAccessFlags = Priv_CommandListVk::locResolveValidateDstAccessMask(aResource, myCommandListType, aNewAccessFlags);

    if (!Priv_CommandListVk::locValidateDstImageLayout(aResource, aNewImageLayout))
      return;

    uint numPossibleSubresourceTransitions = 0u;
    DynamicArray<bool> subresourceTransitionPossible(aSubresourceRange.GetNumSubresources(), false);
    uint i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it)
    {
      const bool transitionPossible = ValidateSubresourceTransition(aResource, aResource->GetSubresourceIndex(*it), dstAccessFlags, aNewImageLayout);
      if (transitionPossible)
        ++numPossibleSubresourceTransitions;
      subresourceTransitionPossible[i++] = transitionPossible;
    }

    if (numPossibleSubresourceTransitions == 0u && canEarlyOut)
      return;

    bool canTransitionAllSubresources = numPossibleSubresourceTransitions == aResource->mySubresources.GetNumSubresources();

    const bool dstIsRead = (dstAccessFlags & Priv_CommandListVk::locAccessMaskRead) == dstAccessFlags;

    LocalHazardData* localData = nullptr;
    auto it = myLocalHazardData.find(aResource);
    if (it == myLocalHazardData.end())  // We don't have a local record of this resource yet
    {
      localData = &myLocalHazardData[aResource];
      localData->mySubresources.resize(aResource->mySubresources.GetNumSubresources());
    }
    else
    {
      localData = &it->second;
    }

    if (!canTransitionAllSubresources && aResource->IsTexture())
    {
      ImageMemoryBarrierData imageBarrier;
      imageBarrier.myImage = aResource->myNativeData.To<GpuResourceDataVk*>()->myImage;
      imageBarrier.myFormat = static_cast<const Texture*>(aResource)->GetProperties().myFormat;
      imageBarrier.myDstAccessMask = dstAccessFlags;
      imageBarrier.myDstLayout = aNewImageLayout;

      SubresourceLocation firstSubresource(*aSubresourceRange.Begin());
      SubresourceLocation lastSubresource(*aSubresourceRange.Begin());

      i = 0u;
      for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
      {
        if (!subresourceTransitionPossible[i])
        {
          if (firstSubresource != lastSubresource)  // Add the barrier we have batched so far since we've reached a discontinuity in subresource-transitions
          {
            imageBarrier.mySubresourceRange = SubresourceRange(firstSubresource, lastSubresource);
            AddBarrier(imageBarrier);
            firstSubresource = lastSubresource;
            lastSubresource = firstSubresource;
          }

          continue;
        }

        const uint subresourceIndex = aResource->GetSubresourceIndex(*it);

        SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
        if (subData.myWasUsed)
        {
          // We can add a barrier. Check if we can append the transition to the current barrier or if we have to start a new one.
          const bool isEmpty = firstSubresource == lastSubresource;
          const bool canBatch = isEmpty || (imageBarrier.mySrcAccessMask == subData.myAccessFlags && imageBarrier.mySrcLayout == subData.myImageLayout);

          if (canBatch)
          {
            imageBarrier.mySrcAccessMask = subData.myAccessFlags;
            imageBarrier.mySrcLayout = subData.myImageLayout;
            lastSubresource = *it;
          }
          else
          {
            imageBarrier.mySubresourceRange = SubresourceRange(firstSubresource, lastSubresource);
            AddBarrier(imageBarrier);
            firstSubresource = lastSubresource;
            lastSubresource = firstSubresource;
          }
        }
      }
    }

    for (uint i = 0u; canTransitionAllSubresources && i < localData->mySubresources.size(); ++i)
      canTransitionAllSubresources &= localData->mySubresources[i].myWasUsed;

    if (canTransitionAllSubresources)
    {
      if (aResource->IsBuffer())
      {
        BufferMemoryBarrierData barrier;
        barrier.myBuffer = aResource->myNativeData.To<GpuResourceDataVk*>()->myBuffer;
        barrier.myDstAccessMask = dstAccessFlags;
        barrier.mySrcAccessMask = localData->mySubresources[0].myAccessFlags;
        AddBarrier(barrier);
      }
      else
      {
        const DataFormat format = static_cast<const Texture*>(aResource)->GetProperties().myFormat;

        ImageMemoryBarrierData barrier;
        barrier.myImage = aResource->myNativeData.To<GpuResourceDataVk*>()->myImage;
        barrier.myFormat = static_cast<const Texture*>(aResource)->GetProperties().myFormat;
        barrier.myDstAccessMask = dstAccessFlags;
        barrier.myDstLayout = aNewImageLayout;
        barrier.mySrcAccessMask = localData->mySubresources[0].myAccessFlags;
        barrier.mySrcLayout = localData->mySubresources[0].myImageLayout;
        barrier.mySubresourceRange = aResource->GetSubresources();
        AddBarrier(barrier);
      }
    }

    i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
    {
      const uint subresourceIndex = aResource->GetSubresourceIndex(*it);
      SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
      if (aIsSharedReadState)
      {
        subData.myWasWritten = false;
        subData.myIsSharedReadState = true;
      }

      if (!subresourceTransitionPossible[i])
        continue;

      if (!subData.myWasUsed)
      {
        subData.myFirstDstAccessFlags = dstAccessFlags;
        subData.myFirstDstImageLayout = aNewImageLayout;
      }

      subData.myWasUsed = true;
      subData.myAccessFlags = dstAccessFlags;
      subData.myImageLayout = aNewImageLayout;

      if (!dstIsRead)
      {
        subData.myWasWritten = true;
        subData.myIsSharedReadState = false;
      }
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::AddBarrier(const BufferMemoryBarrierData& aBarrier)
  {
    if (myPendingBufferBarriers.IsFull())
      FlushBarriers();

    myPendingBufferBarriers.Add(aBarrier);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::AddBarrier(const ImageMemoryBarrierData& aBarrier)
  {
    if (myPendingImageBarriers.IsFull())
      FlushBarriers();

    myPendingImageBarriers.Add(aBarrier);
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, VkAccessFlags aDstAccess, VkImageLayout aDstImageLayout)
  {
    const GpuResourceHazardData& globalData = aResource->GetHazardData();

    VkAccessFlags currAccess = globalData.myVkData.mySubresources[aSubresourceIndex].myAccessMask;
    VkImageLayout currImgLayout = (VkImageLayout) globalData.myVkData.mySubresources[aSubresourceIndex].myImageLayout;

    CommandListType currGlobalContext = globalData.myDx12Data.mySubresources[aSubresourceIndex].myContext;

    auto it = myLocalHazardData.find(aResource);
    if (it != myLocalHazardData.end())
    {
      currAccess = it->second.mySubresources[aSubresourceIndex].myAccessFlags;
      currImgLayout = it->second.mySubresources[aSubresourceIndex].myImageLayout;
    }

    bool currAccessHasAllDstAccessFlags = (currAccess & aDstAccess) == aDstAccess;
    
    if (it != myLocalHazardData.end()   // We can only truly skip this transition if we already have the resource state on the local timeline
      && currAccessHasAllDstAccessFlags
      && currImgLayout == aDstImageLayout) 
      return false;

    const bool dstIsRead = (aDstAccess & Priv_CommandListVk::locAccessMaskRead) == aDstAccess;
    if (dstIsRead && currGlobalContext == CommandListType::SHARED_READ)  // A transition to a write-state will make the resource be owned by this context-type again so we only have to check for a read-transition
    {
      ASSERT(false, "No resource transitions allowed on SHARED_READ context. Resource must be transitioned to a state mask that incorporates all future read-states");
      return false;
    }

    return true;
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::FindShaderResourceInfo(uint64 aNameHash, ShaderResourceInfoVk& aResourceInfoOut) const
  {
    ASSERT(myGraphicsPipelineState.myShaderPipeline != nullptr || myCurrentContext != CommandListType::Graphics);
    ASSERT(myComputePipelineState.myShader != nullptr || myCurrentContext != CommandListType::Compute);

    const DynamicArray<ShaderResourceInfoVk>& shaderResources = myCurrentContext == CommandListType::Graphics ?
      static_cast<ShaderPipelineVk*>(myGraphicsPipelineState.myShaderPipeline.get())->GetResourceInfos() :
      static_cast<const ShaderVk*>(myComputePipelineState.myShader)->GetResourceInfos();

    auto it = std::find_if(shaderResources.begin(), shaderResources.end(), [aNameHash](const ShaderResourceInfoVk& aResourceInfo) {
      return aResourceInfo.myNameHash == aNameHash;
    });

    if (it == shaderResources.end())
    {
      LOG_WARNING("Resource not found in shader");
      return false;
    }

    aResourceInfoOut = *it;
    return true;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyViewportAndClipRect()
  {
    if (myViewportDirty)
    {
      VkViewport viewport;
      viewport.x = static_cast<float>(myViewportParams.x);
      viewport.y = static_cast<float>(myViewportParams.y);
      viewport.width = static_cast<float>(myViewportParams.z);
      viewport.height = static_cast<float>(myViewportParams.w);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      vkCmdSetViewport(myCommandBuffer, 0u, 1u, &viewport);

      myClipRectDirty = true;
      myViewportDirty = false;
    }

    if (myClipRectDirty)
    {
      const int left = static_cast<int>(myClipRect.x);
      const int top = static_cast<int>(myClipRect.y);
      const int right = static_cast<int>(myClipRect.z);
      const int bottom = static_cast<int>(myClipRect.w);

      VkRect2D clipRect;
      clipRect.offset.x = left;
      clipRect.offset.y = top;
      clipRect.extent.width = right - left;
      clipRect.extent.height = bottom - top;

      vkCmdSetScissor(myCommandBuffer, 0u, 1u, &clipRect);

      myClipRectDirty = false;
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyRenderTargets()
  {
    if (!myRenderTargetsDirty)
    {
      ASSERT(myRenderPass != nullptr && myFramebuffer != nullptr);
      return;
    }

    myRenderTargetsDirty = false;

    const uint numRtsToSet = myGraphicsPipelineState.myNumRenderTargets;
    ASSERT(numRtsToSet <= RenderConstants::kMaxNumRenderTargets);
    ASSERT(myCurrentContext == CommandListType::Graphics);
    
    DataFormat rtFormats[RenderConstants::kMaxNumRenderTargets];
    for (uint i = 0u; i < numRtsToSet; ++i)
    {
      TextureView* renderTarget = myRenderTargets[i];
      ASSERT(renderTarget != nullptr);

      rtFormats[i] = renderTarget->GetProperties().myFormat;
    }

    uint64 renderpassHash = MathUtil::ByteHash(reinterpret_cast<uint8*>(rtFormats), sizeof(DataFormat) * numRtsToSet);
    const bool hasDepthStencilTarget = myDepthStencilTarget != nullptr;
    if (hasDepthStencilTarget)
    {
      const TextureViewProperties& dsvProps = myDepthStencilTarget->GetProperties();
      MathUtil::hash_combine(renderpassHash, (uint)dsvProps.myFormat);
      // Read-only DSVs need to produce a different renderpass-hash 
      MathUtil::hash_combine(renderpassHash, dsvProps.myIsDepthReadOnly ? 1u : 0u);
      MathUtil::hash_combine(renderpassHash, dsvProps.myIsStencilReadOnly ? 1u : 0u);
      MathUtil::hash_combine(renderpassHash, (uint)myCurrentContext);
    }

    VkRenderPass renderPass = nullptr;
    auto renderPassIt = ourRenderpassCache.find(renderpassHash);
    if (renderPassIt != ourRenderpassCache.end())
    {
      renderPass = renderPassIt->second;
    }
    else
    {
      renderPass = Priv_CommandListVk::locCreateRenderPass(const_cast<const TextureView**>(myRenderTargets), numRtsToSet, myDepthStencilTarget);
      ourRenderpassCache[renderpassHash] = renderPass;
    }

    uint64 framebufferHash = MathUtil::ByteHash(reinterpret_cast<uint8*>(myRenderTargets), sizeof(TextureView*) * numRtsToSet);
    if (hasDepthStencilTarget)
      MathUtil::hash_combine(framebufferHash, reinterpret_cast<uint64>(myDepthStencilTarget));

    glm::uvec2 framebufferRes(1u, 1u);
    const TextureView* firstRenderTarget = numRtsToSet > 0 ? myRenderTargets[0] : myDepthStencilTarget;
    if (firstRenderTarget != nullptr)
    {
      const TextureProperties& textureProps = firstRenderTarget->GetTexture()->GetProperties();
      framebufferRes.x = textureProps.myWidth;
      framebufferRes.y = textureProps.myHeight;
    }

    VkFramebuffer framebuffer = nullptr;
    auto framebufferIt = ourFramebufferCache.find(framebufferHash);
    if (framebufferIt != ourFramebufferCache.end())
    {
      framebuffer = framebufferIt->second;
    }
    else
    {
      framebuffer = Priv_CommandListVk::locCreateFramebuffer(const_cast<const TextureView**>(myRenderTargets), numRtsToSet, myDepthStencilTarget, framebufferRes, renderPass);
      ourFramebufferCache[framebufferHash] = framebuffer;
    }

    myRenderPass = renderPass;
    myFramebuffer = framebuffer;
    myFramebufferRes = framebufferRes;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyGraphicsPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    const uint64 requestedHash = myGraphicsPipelineState.GetHash();

    VkPipeline pipeline = nullptr;

    const auto cachedPipelineIt = ourPipelineCache.find(requestedHash);
    if (cachedPipelineIt != ourPipelineCache.end())
    {
      pipeline = cachedPipelineIt->second;
    }
    else
    {
      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      pipeline = Priv_CommandListVk::locCreateGraphicsPipeline(myGraphicsPipelineState, myRenderPass);
      ourPipelineCache[requestedHash] = pipeline;
    }
    
    vkCmdBindPipeline(myCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyComputePipelineState()
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyResourceState()
  {
    if (myResourceState.myDescriptorSets.IsEmpty())
      return;

    ASSERT(myCurrentContext != CommandListType::Graphics || myGraphicsPipelineState.myShaderPipeline != nullptr);

    for (uint iSet = 0u; iSet < myResourceState.myDescriptorSets.Size(); ++iSet)
    {
      const ResourceState::DescriptorSet& set = myResourceState.myDescriptorSets[iSet];
      ASSERT(!set.myRanges.IsEmpty());

      VkDescriptorSet vkSet = CreateDescriptorSet(set.myLayout);

      VkWriteDescriptorSet baseWriteInfo = {};
      baseWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      baseWriteInfo.dstSet = vkSet;

      StaticArray<VkWriteDescriptorSet, 64> writeInfos;
      for (uint iRange = 0u; iRange < set.myRanges.Size(); ++iRange)
      {
        const ResourceState::DescriptorRange& range = set.myRanges[iRange];

#if FANCY_RENDERER_DEBUG
        if (!range.myImageInfos.empty()) {
          ASSERT(range.myBufferInfos.empty() && range.myTexelBufferViews.empty());
        }
        else if (!range.myBufferInfos.empty()) {
          ASSERT(range.myImageInfos.empty() && range.myTexelBufferViews.empty());
        }
        else if (!range.myTexelBufferViews.empty()) {
          ASSERT(range.myImageInfos.empty() && range.myBufferInfos.empty());
        }
        ASSERT(!range.myImageInfos.empty() || !range.myBufferInfos.empty() || !range.myBufferInfos.empty(), "Descriptor set %d doesn't have any resources bound on range %d", iSet, iRange);
#endif  // FANCY_RENDERER_DEBUG

        const uint numDescriptors = (uint) glm::max(range.myImageInfos.size(), glm::max(range.myBufferInfos.size(), range.myTexelBufferViews.size()));

        VkWriteDescriptorSet writeInfo = baseWriteInfo;
        writeInfo.descriptorType = range.myType;
        writeInfo.dstArrayElement = 0u;
        writeInfo.descriptorCount = numDescriptors;
        writeInfo.dstBinding = range.myBindingInSet;
        writeInfo.pImageInfo = range.myImageInfos.empty() ? nullptr : range.myImageInfos.data();
        writeInfo.pBufferInfo = range.myBufferInfos.empty() ? nullptr : range.myBufferInfos.data();
        writeInfo.pTexelBufferView = range.myTexelBufferViews.empty() ? nullptr : range.myTexelBufferViews.data();
        writeInfos.Add(writeInfo);
      }

      vkUpdateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, writeInfos.Size(), writeInfos.GetBuffer(), 0u, nullptr);

      VkPipelineBindPoint bindPoint;
      VkPipelineLayout pipelineLayout;
      switch (myCurrentContext)
      {
      case CommandListType::Graphics:
        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 
        pipelineLayout = static_cast<ShaderPipelineVk*>(myGraphicsPipelineState.myShaderPipeline.get())->myPipelineLayout;
        break;
      case CommandListType::Compute:
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE; 
        pipelineLayout = nullptr; // TODO: Add support for compute shaders. Do they even have a pipeline layout?
        break;
      default: ASSERT(false);
      }

      // TODO: Dynamic offsets might be needed for ring-allocated cbuffers
      const uint numDynamicOffsets = 0u;
      const uint* dynamicOffsets = nullptr;
      vkCmdBindDescriptorSets(GetCommandBuffer(), bindPoint, pipelineLayout, set.mySet, 1u, &vkSet, numDynamicOffsets, dynamicOffsets);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearResourceState()
  {
    for (uint i = 0u; i < myResourceState.myTempBufferViews.Size(); ++i)
      RenderCore::GetPlatformVk()->ReleaseTempBufferView(myResourceState.myTempBufferViews[i].first, myResourceState.myTempBufferViews[i].second);
    myResourceState.myTempBufferViews.ClearDiscard();

    myResourceState.myDescriptorSets.Clear();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindInternal(const ShaderResourceInfoVk& aResourceInfo, uint anArrayIndex,  VkBufferView aBufferView,
    VkBuffer aBuffer, uint64 aBufferOffset, uint64 aBufferSize, VkImageView anImageView, VkImageLayout anImageLayout, VkSampler aSampler)
  {
    ASSERT(aResourceInfo.myNumDescriptors > anArrayIndex);

    int iSet = -1;
    for (uint i = 0u; iSet == -1 && i < myResourceState.myDescriptorSets.Size(); ++i)
      if (myResourceState.myDescriptorSets[i].mySet == aResourceInfo.myDescriptorSet)
        iSet = i;

    if (iSet == -1)
    {
      ASSERT(myCurrentContext == CommandListType::Graphics); // TODO: Add compute shader support. Also needs a pipeline layout and descriptor set layouts

      ASSERT(myGraphicsPipelineState.myShaderPipeline != nullptr || myCurrentContext != CommandListType::Graphics);
      ASSERT(myComputePipelineState.myShader != nullptr || myCurrentContext != CommandListType::Compute);

      const VkDescriptorSetLayout setLayout = static_cast<ShaderPipelineVk*>(myGraphicsPipelineState.myShaderPipeline.get())->GetDescriptorSetLayout(aResourceInfo.myDescriptorSet);

      ResourceState::DescriptorSet set;
      set.mySet = aResourceInfo.myDescriptorSet;
      set.myLayout = setLayout;
      myResourceState.myDescriptorSets.Add(set);
      iSet = myResourceState.myDescriptorSets.Size() - 1;
    }

    ResourceState::DescriptorSet& set = myResourceState.myDescriptorSets[iSet];

    ResourceState::DescriptorRange* range = nullptr;
    for (uint i = 0u; i < set.myRanges.Size(); ++i)
    {
      if (set.myRanges[i].myBindingInSet == aResourceInfo.myBindingInSet)
      {
        ASSERT(set.myRanges[i].myType == aResourceInfo.myType);
        range = &set.myRanges[i];
      }
    }

    if (range == nullptr)
    {
      range = &set.myRanges.Add();
      range->myType = aResourceInfo.myType;
      range->myBindingInSet = aResourceInfo.myBindingInSet;
    }

    switch (aResourceInfo.myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
      {
      ASSERT(aSampler != nullptr);
      if (range->myImageInfos.size() < anArrayIndex + 1)
        range->myImageInfos.resize(anArrayIndex + 1);
      VkDescriptorImageInfo& info = range->myImageInfos[anArrayIndex];
      info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      info.imageView = nullptr;
      info.sampler = aSampler;
    } break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    {
      ASSERT(anImageView != nullptr);
      if (range->myImageInfos.size() < anArrayIndex + 1)
        range->myImageInfos.resize(anArrayIndex + 1);
      VkDescriptorImageInfo& info = range->myImageInfos[anArrayIndex];
      info.imageLayout = anImageLayout;
      info.imageView = anImageView;
      info.sampler = nullptr;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {
      ASSERT(aBufferView != nullptr);
      if (range->myTexelBufferViews.size() < anArrayIndex + 1)
        range->myTexelBufferViews.resize(anArrayIndex + 1);
      range->myTexelBufferViews[anArrayIndex] = aBufferView;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
      ASSERT(aBuffer != nullptr && aBufferSize > 0ull);
      if (range->myBufferInfos.size() < anArrayIndex + 1)
        range->myBufferInfos.resize(anArrayIndex + 1);
      VkDescriptorBufferInfo& info = range->myBufferInfos[anArrayIndex];
      info.buffer = aBuffer;
      info.offset = aBufferOffset;
      info.range = aBufferSize;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    default: ASSERT(false);
    }
  }
//---------------------------------------------------------------------------//
  VkDescriptorSet CommandListVk::CreateDescriptorSet(VkDescriptorSetLayout aLayout)
  {
    if (myUsedDescriptorPools.IsEmpty())
      myUsedDescriptorPools.Add(RenderCore::GetPlatformVk()->AllocateDescriptorPool());
    
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &aLayout;
    allocInfo.descriptorPool = myUsedDescriptorPools.GetLast();

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, &allocInfo, &descriptorSet);

    if (result == VK_SUCCESS)
      return descriptorSet;

    myUsedDescriptorPools.Add(RenderCore::GetPlatformVk()->AllocateDescriptorPool());
    allocInfo.descriptorPool = myUsedDescriptorPools.GetLast();
    result = vkAllocateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, &allocInfo, &descriptorSet);

    ASSERT(result == VK_SUCCESS);
    return descriptorSet;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BeginCommandBuffer()
  {
    VkCommandBufferBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    ASSERT_VK_RESULT(vkBeginCommandBuffer(myCommandBuffer, &info));
  }
//---------------------------------------------------------------------------//
}

#endif