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

    uint myDescriptorSetIndex_GlobalResourcesSamplers;
    uint myDescriptorSetIndex_LocalBuffersCBuffers;

    VkPipelineLayout myPipelineLayout;
    VkDescriptorSetLayout myDescriptorSetLayout_GlobalResourcesSamplers;
    VkDescriptorSetLayout myDescriptorSetLayout_LocalBuffersCBuffers;
  };
}

#endif

