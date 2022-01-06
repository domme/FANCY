#pragma once

#if FANCY_ENABLE_VK

#include "RtPipelineState.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  class RtPipelineStateVk final : public RtPipelineState
  {
  public:
    RtPipelineStateVk(const RtPipelineStateProperties& someProps);
    ~RtPipelineStateVk() override;

    bool Recompile() override;

    VkPipeline GetPipeline() const { return myPipeline; }

  protected:
    void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, RtShaderIdentifier& someDataOut);
    void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::ShaderEntry& /*aShaderEntry*/, RtShaderIdentifier& someDataOut) override;
    void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::HitGroup& /*aShaderEntry*/, RtShaderIdentifier& someDataOut) override;

    VkPipeline myPipeline = nullptr;
    eastl::vector<uint8> myShaderHandleStorage;
  };
}


#endif  // FANcyFANCY_ENABLE_VK