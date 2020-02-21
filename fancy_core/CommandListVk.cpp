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
      VkPipelineDynamicStateCreateInfo dynamicStateInfo;
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
      VkPipelineShaderStageCreateInfo pipeShaderCreateInfos[(uint)ShaderStage::NUM_NO_COMPUTE];
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

      VkVertexInputBindingDescription vertexBindingDesc;
      vertexBindingDesc.binding = 0;
      vertexBindingDesc.stride = vertexShader->myVertexAttributeDesc.myOverallVertexSize;
      vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      // TODO: Rework this part so that the user can define how the vertex-binding is to be set up.
      VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
      vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputCreateInfo.pNext = nullptr;
      vertexInputCreateInfo.flags = 0u;
      vertexInputCreateInfo.pVertexAttributeDescriptions = vertexShader->myVertexAttributeDesc.myVertexAttributes.data();
      vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint)vertexShader->myVertexAttributeDesc.myVertexAttributes.size();
      vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDesc;
      vertexInputCreateInfo.vertexBindingDescriptionCount = 1u;

      pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;

      // Input assembly state
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
      inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssemblyCreateInfo.pNext = nullptr;
      inputAssemblyCreateInfo.flags = 0u;
      inputAssemblyCreateInfo.topology = RenderCore_PlatformVk::ResolveTopologyType(aState.myTopologyType);
      inputAssemblyCreateInfo.primitiveRestartEnable = false;
      pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;

      // Multisample state
      VkPipelineMultisampleStateCreateInfo multisampleInfo;
      multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampleInfo.pNext = nullptr;
      multisampleInfo.flags = 0u;
      multisampleInfo.alphaToCoverageEnable = false;
      multisampleInfo.alphaToOneEnable = false;
      multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampleInfo.sampleShadingEnable = false;
      multisampleInfo.minSampleShading = 0.0f;
      VkSampleMask sampleMask = ~0u;
      multisampleInfo.pSampleMask = &sampleMask;
      pipelineCreateInfo.pMultisampleState = &multisampleInfo;

      // Color blend state
      VkPipelineColorBlendStateCreateInfo colorBlendInfo;
      colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlendInfo.pNext = nullptr;
      colorBlendInfo.flags = 0u;

      const BlendStateProperties& blendProps = aState.myBlendState->GetProperties();
      colorBlendInfo.logicOpEnable = blendProps.myLogicOpEnabled;
      colorBlendInfo.logicOp = RenderCore_PlatformVk::ResolveLogicOp(blendProps.myLogicOp);

      VkPipelineColorBlendAttachmentState colorAttachmentBlendState[RenderConstants::kMaxNumRenderTargets];
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
      VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
      const DepthStencilStateProperties& dsProps = aState.myDepthStencilState->GetProperties();
      depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depthStencilInfo.pNext = nullptr;
      depthStencilInfo.flags = 0u;
      depthStencilInfo.depthBoundsTestEnable = false; // TODO: Add support for depthbounds test
      depthStencilInfo.minDepthBounds = 0.0f;  // Will be set as a dynamic state
      depthStencilInfo.maxDepthBounds = FLT_MAX;
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
      VkPipelineRasterizationStateCreateInfo rasterInfo;
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
      VkPipelineTessellationStateCreateInfo tesselationInfo;
      tesselationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
      tesselationInfo.pNext = nullptr;
      tesselationInfo.flags = 0u;
      tesselationInfo.patchControlPoints = 1u;
      pipelineCreateInfo.pTessellationState = &tesselationInfo;

      // Viewport state (will be handled as a dynamic state on the command list)
      VkPipelineViewportStateCreateInfo viewportInfo;
      viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportInfo.pNext = nullptr;
      viewportInfo.flags = 0u;

      // Dummy values - actual viewport will be set as a dynamic state to be more similar to DX
      VkViewport viewport;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = FLT_MAX;
      viewport.width = 1280.0f;
      viewport.height = 720.0f;
      viewport.x = 0.0f;
      viewport.y = 0.0f;

      VkOffset2D offset;
      offset.x = 0;
      offset.y = 0;
      VkExtent2D extend;
      extend.width = 1280u;
      extend.height = 720u;

      VkRect2D scissor;
      scissor.offset = offset;
      scissor.extent = extend;

      viewportInfo.pViewports = &viewport;
      viewportInfo.viewportCount = 1u;
      viewportInfo.pScissors = &scissor;
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
    , myNumPendingBufferBarriers(0u)
    , myNumPendingImageBarriers(0u)
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
    // Need to temporarily transition to WRITE_COPY_DEST or PRESENT so that the image is in the VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR layout.
    // The ATTACHMENT_OPTIMAL layout is not supported with vkCmdClearColorImage

    // Change image layout to GENERAL, keep everyting else untouched
    const ResourceBarrierInfoVk renderTargetBarrierInfo = RenderCore_PlatformVk::ResolveResourceState(GpuResourceState::WRITE_RENDER_TARGET);
    SubresourceBarrierInternal(aTextureView->GetTexture(), aTextureView->mySubresourceRange,
      renderTargetBarrierInfo.myAccessMask,
      renderTargetBarrierInfo.myAccessMask,
      renderTargetBarrierInfo.myStageMask,
      renderTargetBarrierInfo.myStageMask,
      renderTargetBarrierInfo.myImageLayout,
      VK_IMAGE_LAYOUT_GENERAL,
      myCommandListType,
      myCommandListType);

    FlushBarriers();
    
    VkClearColorValue clearColor;
    memcpy(clearColor.float32, aColor, sizeof(clearColor.float32));

    VkImageSubresourceRange subRange = RenderCore_PlatformVk::ResolveSubresourceRange(aTextureView->mySubresourceRange, 
      aTextureView->GetTexture()->GetProperties().myFormat);

    GpuResourceDataVk* dataVk = static_cast<TextureVk*>(aTextureView->GetTexture())->GetData();
    vkCmdClearColorImage(myCommandBuffer, dataVk->myImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1u, &subRange);

    // Change image layout back to the one reported by WRITE_RENDER_TARGET
    SubresourceBarrierInternal(aTextureView->GetTexture(), aTextureView->mySubresourceRange,
      renderTargetBarrierInfo.myAccessMask,
      renderTargetBarrierInfo.myAccessMask,
      renderTargetBarrierInfo.myStageMask,
      renderTargetBarrierInfo.myStageMask,
      VK_IMAGE_LAYOUT_GENERAL,
      renderTargetBarrierInfo.myImageLayout,
      myCommandListType,
      myCommandListType);

    FlushBarriers();
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

    FlushBarriers();

    // Texture is expected to transition to COPY_SRC before using this method
    const VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;  
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

    // Textures are expected to transition to COPY_SRC and COPY_DST before using this method
    const VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    const VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

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

    FlushBarriers();

    const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // Texture is expected in the WRITE_COPY_DST state here
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
  }
//---------------------------------------------------------------------------//
  void CommandListVk::PreBegin()
  {
    CommandList::PreBegin();

    myRenderPass = nullptr;
    myFramebuffer = nullptr;
    myFramebufferRes = glm::uvec2(0u, 0u);
    myNumPendingBufferBarriers = 0u;
    myNumPendingImageBarriers = 0u;

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    myCommandBuffer = platformVk->GetNewCommandBuffer(myCommandListType);
    ASSERT_VK_RESULT(vkResetCommandBuffer(myCommandBuffer, 0u));

    ClearResourceState();

    BeginCommandBuffer();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::FlushBarriers()
  {
    if (myNumPendingImageBarriers == 0u && myNumPendingBufferBarriers == 0u)
      return;

    ASSERT(myPendingBarrierSrcStageMask != 0u && myPendingBarrierDstStageMask != 0u);

    VkImageMemoryBarrier imageBarriersVk[kNumCachedBarriers];
    VkBufferMemoryBarrier bufferBarriersVk[kNumCachedBarriers];

    for (uint i = 0u, e = myNumPendingImageBarriers; i < e; ++i)
    {
      VkImageMemoryBarrier& imageBarrierVk = imageBarriersVk[i];
      const ImageMemoryBarrierData& pendingBarrier = myPendingImageBarriers[i];

      imageBarrierVk.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imageBarrierVk.pNext = nullptr;
      imageBarrierVk.subresourceRange = pendingBarrier.mySubresourceRange;
      imageBarrierVk.image = pendingBarrier.myImage;
      imageBarrierVk.srcAccessMask = pendingBarrier.mySrcAccessMask;
      imageBarrierVk.dstAccessMask = pendingBarrier.myDstAccessMask;
      imageBarrierVk.oldLayout = pendingBarrier.mySrcLayout;
      imageBarrierVk.newLayout = pendingBarrier.myDstLayout;
      imageBarrierVk.srcQueueFamilyIndex = pendingBarrier.mySrcQueueFamilyIndex;
      imageBarrierVk.dstQueueFamilyIndex = pendingBarrier.myDstQueueFamilyIndex;
    }

    for (uint i = 0u, e = myNumPendingBufferBarriers; i < e; ++i)
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

    const VkDependencyFlags dependencyFlags = 0u;
    vkCmdPipelineBarrier(myCommandBuffer, myPendingBarrierSrcStageMask, myPendingBarrierDstStageMask, 
      dependencyFlags, 0u, nullptr, 
      myNumPendingBufferBarriers, bufferBarriersVk, myNumPendingImageBarriers, imageBarriersVk);

    myNumPendingBufferBarriers = 0u;
    myNumPendingImageBarriers = 0u;
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
  void CommandListVk::RenderGeometry(const GeometryData* pGeometry)
  {
    VK_MISSING_IMPLEMENTATION();
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
  void CommandListVk::BindResourceView(const GpuResourceView* aView, uint64 aNameHash)
  {
    ShaderResourceInfoVk resourceInfo;
    if (!FindShaderResourceInfo(aNameHash, resourceInfo))
      return;

#if FANCY_RENDERER_DEBUG
    switch (resourceInfo.myType)
    {
      case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break;  // Needs its own function. Samplers will not be a GpuResourceView
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: 
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        ASSERT(aView->myType == GpuResourceViewType::SRV || aView->myType == GpuResourceViewType::UAV);
        break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        ASSERT(aView->myType == GpuResourceViewType::CBV);
        break;
      default: ASSERT(false);
    }
#endif

    const GpuResourceViewDataVk& viewDataVk = aView->myNativeData.To<GpuResourceViewDataVk>();
    const GpuResourceDataVk* resourceDataVk = aView->myResource->myNativeData.To<GpuResourceDataVk*>();

    int iSet = -1;
    for (uint i = 0u; iSet == -1 && i < myResourceState.myDescriptorSets.Size(); ++i)
      if (myResourceState.myDescriptorSets[i].mySet == resourceInfo.myDescriptorSet)
        iSet = i;

    if (iSet == -1)
    {
      ASSERT(myCurrentContext == CommandListType::Graphics); // TODO: Add compute shader support. Also needs a pipeline layout and descriptor set layouts

      ASSERT(myGraphicsPipelineState.myShaderPipeline != nullptr || myCurrentContext != CommandListType::Graphics);
      ASSERT(myComputePipelineState.myShader != nullptr || myCurrentContext != CommandListType::Compute);

      const VkDescriptorSetLayout setLayout = static_cast<ShaderPipelineVk*>(myGraphicsPipelineState.myShaderPipeline.get())->GetDescriptorSetLayout(resourceInfo.myDescriptorSet);

      ResourceState::DescriptorSet set;
      set.mySet = resourceInfo.myDescriptorSet;
      set.myLayout = setLayout;
      myResourceState.myDescriptorSets.Add(set);
      iSet = myResourceState.myDescriptorSets.Size() - 1;
    }

    ResourceState::DescriptorSet& set = myResourceState.myDescriptorSets[iSet];

    ResourceState::Descriptor* descriptor = nullptr;
    for (uint i = 0u; i < set.myDescriptors.Size(); ++i)
      if (set.myDescriptors[i].myBindingInSet == resourceInfo.myBindingInSet)
        descriptor = &set.myDescriptors[i];

    if (descriptor == nullptr)
      descriptor = &set.myDescriptors.Add();

    switch (aView->myType)
    {
      case GpuResourceViewType::CBV: 
        
        break;
      case GpuResourceViewType::SRV: break;
      case GpuResourceViewType::UAV: break;
      case GpuResourceViewType::DSV: break;
      case GpuResourceViewType::RTV: break;
      default: ;
    }

  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindSampler(const TextureSampler* aSampler, uint64 aNameHash)
  {
    VK_MISSING_IMPLEMENTATION();
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
  bool CommandListVk::SubresourceBarrierInternal(
    const GpuResource* aResource,
    const SubresourceRange& aSubresourceRange,
    GpuResourceState aSrcState,
    GpuResourceState aDstState,
    CommandListType aSrcQueue,
    CommandListType aDstQueue)
  {
    const ResourceBarrierInfoVk srcBarrierInfo = RenderCore_PlatformVk::ResolveResourceState(aSrcState);
    const ResourceBarrierInfoVk dstBarrierInfo = RenderCore_PlatformVk::ResolveResourceState(aDstState);
    
    return SubresourceBarrierInternal(
      aResource, 
      aSubresourceRange,
      srcBarrierInfo.myAccessMask,
      dstBarrierInfo.myAccessMask,
      srcBarrierInfo.myStageMask,
      dstBarrierInfo.myStageMask,
      srcBarrierInfo.myImageLayout,
      dstBarrierInfo.myImageLayout,
      aSrcQueue,
      aDstQueue);
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::SubresourceBarrierInternal(const GpuResource* aResource,
    const SubresourceRange& aSubresourceRange,
    VkAccessFlags aSrcAccessMask, 
    VkAccessFlags aDstAccessMask, 
    VkPipelineStageFlags aSrcStageMask,
    VkPipelineStageFlags aDstStageMask, 
    VkImageLayout aSrcImageLayout, 
    VkImageLayout aDstImageLayout,
    CommandListType aSrcQueue, 
    CommandListType aDstQueue)
  {
    GpuResourceStateTracking& resourceStateTracking = aResource->myStateTracking;
    if (!resourceStateTracking.myCanChangeStates)
      return false;

    constexpr VkAccessFlags accessMaskGraphics = static_cast<D3D12_RESOURCE_STATES>(~0u);
    constexpr VkAccessFlags accessMaskCompute = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT
      | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT
      | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    constexpr VkAccessFlags accessMaskRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
      VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX | VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT | VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;

    constexpr VkAccessFlags accessMaskWrite = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT |
      VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT | VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;

    const bool srcIsRead = (aSrcAccessMask & accessMaskRead) != 0u;
    ASSERT(!srcIsRead || (aSrcAccessMask & accessMaskWrite) == 0u, "src access mask contains both read and write bits");

    const bool dstIsRead = (aDstAccessMask & accessMaskRead) != 0u;
    ASSERT(!dstIsRead || (aDstAccessMask & accessMaskWrite) == 0u, "dst access mask contains both read and write bits");

    const VkAccessFlags accessMaskSrcQueue = aSrcQueue == CommandListType::Graphics ? accessMaskGraphics : accessMaskCompute;
    const VkAccessFlags accessMaskDstQueue = aDstQueue == CommandListType::Graphics ? accessMaskGraphics : accessMaskCompute;
    const VkAccessFlags accessMaskCurrentQueue = myCommandListType == CommandListType::Graphics ? accessMaskGraphics : accessMaskCompute;

    constexpr VkPipelineStageFlags pipelineMaskGraphics = static_cast<D3D12_RESOURCE_STATES>(~0u);
    constexpr VkPipelineStageFlags pipelineMaskCompute = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    const VkPipelineStageFlags pipelineMaskSrcQueue = aSrcQueue == CommandListType::Graphics ? pipelineMaskGraphics : pipelineMaskCompute;
    const VkPipelineStageFlags pipelineMaskDstQueue = aDstQueue == CommandListType::Graphics ? pipelineMaskGraphics : pipelineMaskCompute;
    const VkPipelineStageFlags pipelineMaskCurrentQueue = myCommandListType == CommandListType::Graphics ? pipelineMaskGraphics : pipelineMaskCompute;

    const VkPipelineStageFlags srcPipelineMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | (pipelineMaskSrcQueue & pipelineMaskCurrentQueue & aSrcStageMask);
    const VkPipelineStageFlags dstPipelineMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | (pipelineMaskDstQueue & pipelineMaskCurrentQueue & aDstStageMask);

    const VkAccessFlags resourceWriteAccessMask = static_cast<VkAccessFlags>(resourceStateTracking.myVkData.myWriteAccessMask);
    const VkAccessFlags resourceReadAccessMask = static_cast<VkAccessFlags>(resourceStateTracking.myVkData.myReadAccessMask);

    const VkAccessFlags allowedSrcAccessMask = aSrcAccessMask & (srcIsRead ? resourceReadAccessMask : resourceWriteAccessMask);
    const VkAccessFlags allowedDstAccessMask = aDstAccessMask & (dstIsRead ? resourceReadAccessMask : resourceWriteAccessMask);
    const VkAccessFlags srcAccessMask = allowedSrcAccessMask & accessMaskSrcQueue & accessMaskCurrentQueue;
    const VkAccessFlags dstAccessMask = allowedDstAccessMask & accessMaskDstQueue & accessMaskCurrentQueue;
    ASSERT((aSrcAccessMask == 0u || srcAccessMask != 0u) && (aDstAccessMask == 0u || dstAccessMask != 0u), "Invalid barrier");

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const uint srcQueueFamilyIndex = 
      resourceStateTracking.myVkData.myHasExclusiveQueueAccess ? platformVk->GetQueueInfo(aSrcQueue).myQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;
    const uint dstQueueFamilyIndex = 
      resourceStateTracking.myVkData.myHasExclusiveQueueAccess ? platformVk->GetQueueInfo(aDstQueue).myQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;

    const bool isImage = aResource->myCategory == GpuResourceCategory::TEXTURE;
    GpuResourceDataVk* dataVk = aResource->myNativeData.To<GpuResourceDataVk*>();

    if (myNumPendingImageBarriers >= kNumCachedBarriers || myNumPendingBufferBarriers >= kNumCachedBarriers)
      FlushBarriers();

    if (isImage)
    {
      const GpuResourceStateTrackingVk& stateTrackingVk = aResource->myStateTracking.myVkData;
      const VkImageLayout srcImageLayout = stateTrackingVk.myHasInitialImageLayout ? static_cast<VkImageLayout>(stateTrackingVk.myInitialImageLayout) : aSrcImageLayout;
      stateTrackingVk.myHasInitialImageLayout = false;

      ImageMemoryBarrierData& imageBarrier = myPendingImageBarriers[myNumPendingImageBarriers++];
      imageBarrier.myImage = dataVk->myImage;
      imageBarrier.mySrcAccessMask = srcAccessMask;
      imageBarrier.myDstAccessMask = dstAccessMask;
      imageBarrier.mySrcLayout = srcImageLayout;
      imageBarrier.myDstLayout = aDstImageLayout;
      imageBarrier.mySrcQueueFamilyIndex = srcQueueFamilyIndex;
      imageBarrier.myDstQueueFamilyIndex = dstQueueFamilyIndex;

      const TextureVk* texture = static_cast<const TextureVk*>(aResource);
      imageBarrier.mySubresourceRange = RenderCore_PlatformVk::ResolveSubresourceRange(aSubresourceRange, texture->GetProperties().myFormat);
    }
    else
    {
      BufferMemoryBarrierData& bufferBarrier = myPendingBufferBarriers[myNumPendingBufferBarriers++];
      bufferBarrier.myBuffer = dataVk->myBuffer;
      bufferBarrier.mySrcAccessMask = srcAccessMask;
      bufferBarrier.myDstAccessMask = dstAccessMask;
      bufferBarrier.mySrcQueueFamilyIndex = srcQueueFamilyIndex;
      bufferBarrier.myDstQueueFamilyIndex = dstQueueFamilyIndex;
    }

    myPendingBarrierSrcStageMask |= srcPipelineMask;
    myPendingBarrierDstStageMask |= dstPipelineMask;

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
      LOG_WARNING("Resource %s not found in shader");
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
      ASSERT(!set.myDescriptors.IsEmpty());

      VkDescriptorSet vkSet = CreateDescriptorSet(set.myLayout);

      VkWriteDescriptorSet baseWriteInfo = {};
      baseWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      baseWriteInfo.dstSet = vkSet;
      
      StaticArray<VkWriteDescriptorSet, 64> writeInfos;
      for (uint iDesc = 0u; iDesc < set.myDescriptors.Size(); ++iDesc)
      {
        const ResourceState::Descriptor& desc = set.myDescriptors[iDesc];

        VkWriteDescriptorSet writeInfo = baseWriteInfo;
        writeInfo.descriptorType = desc.myType;
        writeInfo.dstArrayElement = desc.myFirstArrayElement;
        writeInfo.descriptorCount = desc.myNumDescriptors;
        writeInfo.dstBinding = desc.myBindingInSet;
        writeInfo.pImageInfo = desc.myImageInfos;
        writeInfo.pBufferInfo = desc.myBufferInfos;
        writeInfo.pTexelBufferView = desc.myTexelBufferView;
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
    myResourceState.myDescriptorSets.ClearDiscard();
    myResourceState.myBoundBuffers.ClearDiscard();
    myResourceState.myBoundImages.ClearDiscard();
    myResourceState.myBoundTexelBufferView.ClearDiscard();
  }
//---------------------------------------------------------------------------//
  VkDescriptorSet CommandListVk::CreateDescriptorSet(VkDescriptorSetLayout aLayout)
  {
    if (myUsedDescriptorPools.IsEmpty())
      myUsedDescriptorPools.Add(RenderCore::GetPlatformVk()->AllocateDescriptorPool());
    
    VkDescriptorSetAllocateInfo allocInfo;
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
