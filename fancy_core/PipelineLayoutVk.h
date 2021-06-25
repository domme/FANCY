#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  /// <summary>
  /// Represents the one single pipeline layout used by Fancy that is shared accross all shaders
  /// </summary>

  struct PipelineLayoutVk
  {
    PipelineLayoutVk(const RenderPlatformProperties& someProperties);
    ~PipelineLayoutVk();

    uint myDescriptorSetIndex_LocalBuffers;
    uint myDescriptorSetIndex_LocalRwBuffers;
    uint myDescriptorSetIndex_LocalCbuffers;
    uint myDescriptorSetIndex_GlobalResourcesSamplers;

    VkDescriptorType myDescriptorType_LocalBuffers;
    VkDescriptorType myDescriptorType_LocalRwBuffers;
    VkDescriptorType myDescriptorType_LocalCBuffers;

    VkPipelineLayout myPipelineLayout;
    VkDescriptorSetLayout myDescriptorSetLayout_GlobalResourcesSamplers;
    VkDescriptorSetLayout myDescriptorSetLayout_LocalBuffers;
    VkDescriptorSetLayout myDescriptorSetLayout_LocalRwBuffers;
    VkDescriptorSetLayout myDescriptorSetLayout_LocalCbuffers;
  };
}

#endif

