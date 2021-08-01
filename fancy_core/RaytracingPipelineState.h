#pragma once

namespace Fancy
{
  struct RaytracingPipelineStateProperties
  {
    void SetRayGenShader(const SharedPtr<Shader>& aShader);
    void SetMissShader(const SharedPtr<Shader>& aShader);
    uint AddHitGroup(const char* aName, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader, RaytracingHitGroupType aType);
    uint64 GetHash() const;
    uint AddUniqueHitShaderGetIndex(const SharedPtr<Shader>& aShader);
    void SetMaxPayloadSize(uint aSizeBytes) { myMaxPayloadSizeBytes = aSizeBytes; }
    void SetMaxAttributeSize(uint aSizeBytes) { myMaxAttributeSizeBytes = aSizeBytes; }
    void SetMaxRecursionDepth(uint aMaxDepth) { myMaxRecursionDepth = aMaxDepth; }
    void AddPipelineFlag(RaytracingPipelineFlags aFlag) { myPipelineFlags = RaytracingPipelineFlags(myPipelineFlags | aFlag); }

    struct HitShader
    {
      eastl::string myUniqueMainFunctionName;
      SharedPtr<Shader> myShader;
    };

    struct HitGroup
    {
      RaytracingHitGroupType myType;
      eastl::string myName;
      uint myIntersectionShaderIdx;
      uint myAnyHitShaderIdx;
      uint myClosestHitShaderIdx;
    };

    eastl::vector<HitGroup> myHitGroups;
    eastl::vector<HitShader> myHitShaders;
    SharedPtr<Shader> myRaygenShader;
    SharedPtr<Shader> myMissShader;
    uint myMaxPayloadSizeBytes = sizeof(glm::float4);
    uint myMaxAttributeSizeBytes = 32;
    uint myMaxRecursionDepth = 1;
    RaytracingPipelineFlags myPipelineFlags = RT_PIPELINE_FLAG_NONE;
  };

  class RaytracingPipelineState
  {
  public:
    RaytracingPipelineState(const RaytracingPipelineStateProperties& someProps)
      : myProperties(someProps) { }

    virtual ~RaytracingPipelineState();
    virtual bool Recompile() = 0;

    RaytracingPipelineStateProperties myProperties;
  };
}



