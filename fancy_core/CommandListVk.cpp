#include "fancy_core_precompile.h"
#include "CommandListVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderPipeline.h"
#include "ShaderVk.h"
#include "ShaderPipelineVk.h"
#include "BlendState.h"
#include "DepthStencilState.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandListVk::CommandListVk(CommandListType aType) 
    : CommandList(aType)
    , myIsOpen(true)
    , myCommandBuffer(nullptr)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkCommandPool commandPool = platformVk->GetCommandPool(aType);

    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandBufferCount = 1u;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool;
    ASSERT_VK_RESULT(vkAllocateCommandBuffers(platformVk->myDevice, &allocateInfo, &myCommandBuffer));
  }
//---------------------------------------------------------------------------//
  CommandListVk::~CommandListVk()
  {
    CommandListVk::ReleaseGpuResources(0ull);

    
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyResource(GpuResource* aDestResource, GpuResource* aSrcResource)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyBufferRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTextureRegion(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation, const TextureRegion* aSrcRegion)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, 
    const glm::uvec3& aDestTexelPos, const Texture* aSrcTexture, const TextureSubLocation& aSrcSubLocation,const TextureRegion* aSrcRegion)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTextureRegion(const Texture* aDestTexture, const TextureSubLocation& aDestSubLocation, const glm::uvec3& aDestTexelPos, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ReleaseGpuResources(uint64 aFenceVal)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Reset()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::FlushBarriers()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::SetShaderPipeline(const SharedPtr<ShaderPipeline>& aShaderPipeline)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindVertexBuffer(const GpuBuffer* aBuffer, uint aVertexSize, uint64 anOffset, uint64 aSize)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 aSize)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance)
  {
    FlushBarriers();
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyTopologyType();
    ApplyGraphicsPipelineState();

    vkCmdDrawIndexed(myCommandBuffer, aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::RenderGeometry(const GeometryData* pGeometry)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint aRegisterIndex) const
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindResourceSet(const GpuResourceView** someResourceViews, uint aResourceCount, uint aRegisterIndex)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::BeginQuery(GpuQueryType aType)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
    return GpuQuery(GpuQueryType::TIMESTAMP, 0u, 0ull, myCommandListType);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::EndQuery(const GpuQuery& aQuery)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::InsertTimestamp()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Close()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::IsOpen() const
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::SetComputeProgram(const Shader* aProgram)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Dispatch(const glm::int3& aNumThreads)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::SubresourceBarrierInternal(
    const GpuResource* aResource, 
    const uint16* someSubresources,
    uint aNumSubresources, 
    GpuResourceState aSrcState, 
    GpuResourceState aDstState, 
    CommandListType aSrcQueue,
    CommandListType aDstQueue)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void CommandListVk::ApplyViewportAndClipRect()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void CommandListVk::ApplyRenderTargets()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

  void CommandListVk::ApplyTopologyType()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

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

      VkGraphicsPipelineCreateInfo createInfo = GetGraphicsPipelineCreateInfo(myGraphicsPipelineState);
      ASSERT_VK_RESULT(vkCreateGraphicsPipelines(platformVk->myDevice, VK_NULL_HANDLE, 1u, &createInfo, nullptr, &pipeline));

      ourPipelineCache[requestedHash] = pipeline;
    }
    vkCmdBindPipeline(myCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  void CommandListVk::ApplyComputePipelineState()
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }
//---------------------------------------------------------------------------//
  VkPipeline CommandListVk::CreateGraphicsPipeline(const GraphicsPipelineState& aState, VkRenderPass aRenderPass)
  {
    ASSERT(aState.myNumRenderTargets <= RenderConstants::kMaxNumRenderTargets);
    ASSERT(aState.myNumRenderTargets > 0 || aState.myDSVformat != DataFormat::NONE);
    ASSERT(aState.myShaderPipeline != nullptr);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.renderPass = aRenderPass;

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
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    depthStencilInfo.minDepthBounds = 0.0f;
    depthStencilInfo.maxDepthBounds = FLT_MAX;
    depthStencilInfo.depthTestEnable = dsProps.myDepthTestEnabled;
    depthStencilInfo.depthWriteEnable = dsProps.myDepthWriteEnabled;
    depthStencilInfo.depthCompareOp = RenderCore_PlatformVk::ResolveCompFunc(dsProps.myDepthCompFunc);
    
    // Front face
    {
      VkStencilOpState& faceState = depthStencilInfo.front;
      const DepthStencilFaceProperties& faceProps = dsProps.myFrontFace;
      faceState.writeMask = dsProps.myStencilWriteMask;
      faceState.compareMask = dsProps.myStencilReadMask;
      faceState.reference
    }




    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;

  }

  VkComputePipelineCreateInfo CommandListVk::GetComputePipelineCreateInfo(const ComputePipelineState& aState)
  {
    ASSERT(!VK_ASSERT_MISSING_IMPLEMENTATION, "Not implemented");
  }

//---------------------------------------------------------------------------//
}
