#include "fancy_core_precompile.h"
#include "RtPipelineStateVk.h"

#include "PipelineLayoutVk.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderVk.h"

using namespace Fancy;

namespace Private
{
  //---------------------------------------------------------------------------//

  struct RtPipelineBuilder
  {
    RtPipelineBuilder(uint aNumShaderStages, uint aNumShaderGroups)
    {
      myShaderStages.reserve(aNumShaderStages);
      myShaderGroups.reserve(aNumShaderGroups);
    }

    void AddGeneralShaderGroup(const Shader* aShader)
    {
      const ShaderVk* shaderVk = static_cast<const ShaderVk*>(aShader);
      myShaderStages.push_back(shaderVk->GetStageCreateInfo());

      VkRayTracingShaderGroupCreateInfoKHR& shaderGroupInfo = myShaderGroups.push_back();
      shaderGroupInfo = { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
      shaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
      shaderGroupInfo.generalShader = (uint)myShaderStages.size() - 1;
      shaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
      shaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
      shaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
    }

    void AddHitShaderGroup(const Shader* aClosestHitShader, const Shader* anAnyHitShader, const Shader* anIntersectionShader)
    {
      ASSERT(aClosestHitShader || anAnyHitShader || anIntersectionShader);

      const ShaderVk* closestHitShaderVk = static_cast<const ShaderVk*>(aClosestHitShader);
      const ShaderVk* anyHitShaderVk = static_cast<const ShaderVk*>(anAnyHitShader);
      const ShaderVk* intersectionShaderVk = static_cast<const ShaderVk*>(anIntersectionShader);

      VkRayTracingShaderGroupCreateInfoKHR& shaderGroupInfo = myShaderGroups.push_back();
      shaderGroupInfo = { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
      shaderGroupInfo.type = intersectionShaderVk ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
      shaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;

      shaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
      if (closestHitShaderVk)
      {
        myShaderStages.push_back(closestHitShaderVk->GetStageCreateInfo());
        shaderGroupInfo.closestHitShader = (uint)myShaderStages.size() - 1;
      }

      shaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
      if (anyHitShaderVk)
      {
        myShaderStages.push_back(anyHitShaderVk->GetStageCreateInfo());
        shaderGroupInfo.anyHitShader = (uint)myShaderStages.size() - 1;
      }

      shaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
      if (intersectionShaderVk)
      {
        myShaderStages.push_back(intersectionShaderVk->GetStageCreateInfo());
        shaderGroupInfo.intersectionShader = (uint)myShaderStages.size() - 1;
      }
    }

    bool Build(uint aMaxPayloadSize, uint aMaxAttributeSize, uint aMaxRecursionDepth, uint someRtPipelineFlags, VkPipeline& aPipelineOut, eastl::vector<uint8>& aShaderHandleStorageOut)
    {
      ASSERT(!myShaderStages.empty() && !myShaderGroups.empty());

      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo { VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
      pipelineCreateInfo.stageCount = (uint)myShaderStages.size();
      pipelineCreateInfo.pStages = myShaderStages.data();
      pipelineCreateInfo.groupCount = (uint)myShaderGroups.size();
      pipelineCreateInfo.pGroups = myShaderGroups.data();
      pipelineCreateInfo.maxPipelineRayRecursionDepth = aMaxRecursionDepth;
      pipelineCreateInfo.layout = platformVk->GetPipelineLayout()->myPipelineLayout;
      if (someRtPipelineFlags & RT_PIPELINE_FLAG_SKIP_TRIANGLES)
        pipelineCreateInfo.flags |= VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR;
      if (someRtPipelineFlags & RT_PIPELINE_FLAG_SKIP_PROCEDURAL)
        pipelineCreateInfo.flags |= VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR;

      VkRayTracingPipelineInterfaceCreateInfoKHR pipelineInterfaceInfo{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR };
      pipelineInterfaceInfo.maxPipelineRayHitAttributeSize = aMaxAttributeSize;
      pipelineInterfaceInfo.maxPipelineRayPayloadSize = aMaxPayloadSize;
      pipelineCreateInfo.pLibraryInterface = &pipelineInterfaceInfo;

      VkPipeline pipeline = nullptr;
      VkResult result = VkExt::vkCreateRayTracingPipelinesKHR(platformVk->GetDevice(), nullptr, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
      ASSERT_VK_RESULT(result);

      if (result != VK_SUCCESS)
        return false;
      
      const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
      const uint handleSize = caps.myRaytracingShaderIdentifierSizeBytes;
      const uint alignedHandleSize = (uint) MathUtil::Align(handleSize, caps.myRaytracingShaderRecordAlignment);
      const uint shaderHandleStorageSize = alignedHandleSize * (uint) myShaderGroups.size();

      eastl::vector<uint8> shaderHandleStorage(shaderHandleStorageSize);
      result = VkExt::vkGetRayTracingShaderGroupHandlesKHR(platformVk->GetDevice(), pipeline, 0, (uint)myShaderGroups.size(), shaderHandleStorageSize, shaderHandleStorage.data());
      ASSERT_VK_RESULT(result);

      if (result != VK_SUCCESS)
        return false;

      aPipelineOut = pipeline;
      aShaderHandleStorageOut = eastl::move(shaderHandleStorage);

      return true;
    }

    eastl::vector<VkPipelineShaderStageCreateInfo> myShaderStages;
    eastl::vector<VkRayTracingShaderGroupCreateInfoKHR> myShaderGroups;
  };

  const Shader* GetHitShader(const RtPipelineStateProperties& someProperties, uint aHitShaderIndex)
  {
    if (aHitShaderIndex >= (uint)someProperties.myHitShaders.size())
      return nullptr;

    return someProperties.myHitShaders[aHitShaderIndex].myShader.get();
  }
}

//---------------------------------------------------------------------------//

RtPipelineStateVk::RtPipelineStateVk(const RtPipelineStateProperties& someProps)
  : RtPipelineState(someProps)
{
  Recompile();
}

RtPipelineStateVk::~RtPipelineStateVk()
{
  RenderCore::WaitForIdle(CommandListType::Graphics);
  RenderCore::WaitForIdle(CommandListType::Compute);

  vkDestroyPipeline(RenderCore::GetPlatformVk()->GetDevice(), myPipeline, nullptr);
}

bool RtPipelineStateVk::Recompile()
{
  ASSERT(!myProperties.myRaygenShaders.empty(), "Raygen shader is required to build an RT PSO");

  if (myPipeline)
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    vkDestroyPipeline(RenderCore::GetPlatformVk()->GetDevice(), myPipeline, nullptr);
    myPipeline = nullptr;
  }

  const uint numShaderStages = (uint)myProperties.myRaygenShaders.size() + (uint)myProperties.myMissShaders.size() + (uint)myProperties.myHitShaders.size();
  const uint numShaderGroups = (uint)myProperties.myRaygenShaders.size() + (uint)myProperties.myMissShaders.size() + // Each raygen and miss shader will become its own group with all other shader-stages marked as unused
    +(uint)myProperties.myHitGroups.size();

  Private::RtPipelineBuilder builder(numShaderStages, numShaderGroups);

  for (const RtPipelineStateProperties::ShaderEntry& shaderEntry : myProperties.myRaygenShaders)
    builder.AddGeneralShaderGroup(shaderEntry.myShader.get());

  for (const RtPipelineStateProperties::ShaderEntry& shaderEntry : myProperties.myMissShaders)
    builder.AddGeneralShaderGroup(shaderEntry.myShader.get());

  for (const RtPipelineStateProperties::HitGroup& hitGroup : myProperties.myHitGroups)
  {
    builder.AddHitShaderGroup(
      Private::GetHitShader(myProperties, hitGroup.myClosestHitShaderIdx),
      Private::GetHitShader(myProperties, hitGroup.myAnyHitShaderIdx),
      Private::GetHitShader(myProperties, hitGroup.myIntersectionShaderIdx));
  }

  if (!builder.Build(myProperties.myMaxPayloadSizeBytes, myProperties.myMaxAttributeSizeBytes, myProperties.myMaxRecursionDepth, myProperties.myPipelineFlags, myPipeline, myShaderHandleStorage))
  {
    LOG_ERROR("Failed building Rt Pipeline state");
    return false;
  }

  return true;
}

void RtPipelineStateVk::GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, RtShaderIdentifier& someDataOut)
{
  const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
  const uint handleSize = caps.myRaytracingShaderIdentifierSizeBytes;
  const uint alignedHandleSize = (uint) MathUtil::Align(handleSize, caps.myRaytracingShaderRecordAlignment);

  const uint readOffsetBytes = aShaderIndexInRtPso * alignedHandleSize;
  ASSERT(myShaderHandleStorage.size() >= readOffsetBytes + handleSize);

  someDataOut.myData.resize(handleSize);
  memcpy(someDataOut.myData.data(), myShaderHandleStorage.data() + readOffsetBytes, handleSize);
}

void RtPipelineStateVk::GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::ShaderEntry& /*aShaderEntry*/, RtShaderIdentifier& someDataOut)
{
  GetShaderIdentifierDataInternal(aShaderIndexInRtPso, someDataOut);
}

void RtPipelineStateVk::GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::HitGroup& /*aShaderEntry*/, RtShaderIdentifier& someDataOut)
{
  GetShaderIdentifierDataInternal(aShaderIndexInRtPso, someDataOut);
}

