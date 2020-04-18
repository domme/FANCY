#pragma once



namespace Fancy
{
  class PipelineLayoutCacheVk
  {
  public:
    PipelineLayoutCacheVk();
    ~PipelineLayoutCacheVk();

    struct DescriptorSetLayoutInfo
    {
      DescriptorSetLayoutInfo(uint aSetIndex) : mySet(aSetIndex) { }
      uint mySet;
      DynamicArray<VkDescriptorSetLayoutBinding> myBindings;
    };

  };
}



