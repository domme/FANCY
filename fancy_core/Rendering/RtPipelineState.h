#pragma once

#include "RtShaderIdentifier.h"

namespace Fancy
{
  class Shader;

  class RtPipelineStateProperties
  {
  public:
    uint AddRayGenShader(const char* aPath, const char* aMainFunction, const char* someDefines = nullptr);
    uint AddRayGenShader(const SharedPtr<Shader>& aShader);
    uint AddMissShader(const char* aPath, const char* aMainFunction, const char* someDefines = nullptr);
    uint AddMissShader(const SharedPtr<Shader>& aShader);
    uint AddHitGroup(const wchar_t* aName, RtHitGroupType aType, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader);
    uint AddHitGroup(const wchar_t* aName, RtHitGroupType aType,
      const char* anIntersectionPath, const char* anIntersectionMainFunction,
      const char* anAnyHitPath, const char* anAnyHitMainFunction,
      const char* aClosestHitPath, const char* aClosestHitMainFunction,
      const char* someDefines = nullptr);
    void SetMaxPayloadSize(uint aSizeBytes) { myMaxPayloadSizeBytes = aSizeBytes; }
    void SetMaxAttributeSize(uint aSizeBytes) { myMaxAttributeSizeBytes = aSizeBytes; }
    void SetMaxRecursionDepth(uint aMaxDepth) { myMaxRecursionDepth = aMaxDepth; }
    void AddPipelineFlag(RtPipelineFlags aFlag) { myPipelineFlags = RtPipelineFlags(myPipelineFlags | aFlag); }
    uint64 GetHash() const;
    
    struct ShaderEntry
    {
      eastl::wstring myUniqueMainFunctionName;
      SharedPtr<Shader> myShader;
    };

    struct HitGroup
    {
      RtHitGroupType myType;
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
    RtPipelineFlags myPipelineFlags = RT_PIPELINE_FLAG_NONE;
  };

  class RtPipelineState
  {
  public:
    RtPipelineState(const RtPipelineStateProperties& someProps)
      : myProperties(someProps) { }

    virtual ~RtPipelineState() {};
    virtual bool Recompile() = 0;

    bool HasShader(const Shader* aShader) const;

    RtShaderIdentifier GetRayGenShaderIdentifier(uint anIndex);
    RtShaderIdentifier GetMissShaderIdentifier(uint anIndex);
    RtShaderIdentifier GetHitShaderIdentifier(uint anIndex);

    RtPipelineStateProperties myProperties;

  protected:
    virtual void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::ShaderEntry& aShaderEntry, RtShaderIdentifier& someDataOut) = 0;
    virtual void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::HitGroup& aShaderEntry, RtShaderIdentifier& someDataOut) = 0;
  };
}



