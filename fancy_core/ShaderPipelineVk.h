#pragma once
#include "ShaderPipeline.h"
#include "VkPrerequisites.h"
#include "PipelineLayoutVk.h"
#include "eastl/vector.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    ShaderPipelineVk();
    ~ShaderPipelineVk() override;

    void CreateFromShaders() override;

    const eastl::vector<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    PipelineLayoutVk* GetPipelineLayout() const { return myPipelineLayout.get(); }
    bool HasDescriptorSet(uint aSetIdx) const { return myPipelineLayout->myDescriptorSets.size() > aSetIdx && 
      myPipelineLayout->myDescriptorSets[aSetIdx].myLayout != nullptr; }

  private:
    SharedPtr<PipelineLayoutVk> myPipelineLayout;
    eastl::vector<ShaderResourceInfoVk> myResourceInfos;
  };
}

#endif