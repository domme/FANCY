#pragma once

#include "RaytracingShaderIdentifier.h"

namespace Fancy
{
  class RaytracingPipelineStateProperties
  {
  public:
    uint AddRayGenShader(const char* aPath, const char* aMainFunction, const char* someDefines = nullptr);
    uint AddRayGenShader(const SharedPtr<Shader>& aShader);
    uint AddMissShader(const char* aPath, const char* aMainFunction, const char* someDefines = nullptr);
    uint AddMissShader(const SharedPtr<Shader>& aShader);
    uint AddHitGroup(const wchar_t* aName, RaytracingHitGroupType aType, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader);
    uint AddHitGroup(const wchar_t* aName, RaytracingHitGroupType aType,
      const char* anIntersectionPath, const char* anIntersectionMainFunction,
      const char* anAnyHitPath, const char* anAnyHitMainFunction,
      const char* aClosestHitPath, const char* aClosestHitMainFunction,
      const char* someDefines = nullptr);
    void SetMaxPayloadSize(uint aSizeBytes) { myMaxPayloadSizeBytes = aSizeBytes; }
    void SetMaxAttributeSize(uint aSizeBytes) { myMaxAttributeSizeBytes = aSizeBytes; }
    void SetMaxRecursionDepth(uint aMaxDepth) { myMaxRecursionDepth = aMaxDepth; }
    void AddPipelineFlag(RaytracingPipelineFlags aFlag) { myPipelineFlags = RaytracingPipelineFlags(myPipelineFlags | aFlag); }
    uint64 GetHash() const;


    struct ShaderEntry
    {
      eastl::wstring myUniqueMainFunctionName;
      SharedPtr<Shader> myShader;
    };

    struct HitGroup
    {
      RaytracingHitGroupType myType;
      eastl::wstring myName;
      uint myIntersectionShaderIdx;
      uint myAnyHitShaderIdx;
      uint myClosestHitShaderIdx;
    };

    eastl::vector<HitGroup> myHitGroups;
    eastl::vector<ShaderEntry> myHitShaders;
    eastl::vector<ShaderEntry> myMissShaders;
    eastl::vector<ShaderEntry> myRaygenShaders;
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

    virtual ~RaytracingPipelineState() {};
    virtual bool Recompile() = 0;

    bool HasShader(const Shader* aShader) const;

    RaytracingShaderIdentifier GetRayGenShaderIdentifier(uint anIndex);
    RaytracingShaderIdentifier GetMissShaderIdentifier(uint anIndex);
    RaytracingShaderIdentifier GetHitShaderIdentifier(uint anIndex);

    RaytracingPipelineStateProperties myProperties;

  protected:
    virtual void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry, RaytracingShaderIdentifier& someDataOut) = 0;
    virtual void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aShaderEntry, RaytracingShaderIdentifier& someDataOut) = 0;
  };
}



