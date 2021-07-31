#include "fancy_core_precompile.h"
#include "RaytracingPipelineState.h"

#include "Shader.h"

namespace Fancy
{
  void RaytracingPipelineStateProperties::SetRayGenShader(const SharedPtr<Shader>& aShader)
  {
    myRaygenShader = aShader;
  }

  void RaytracingPipelineStateProperties::SetMissShader(const SharedPtr<Shader>& aShader)
  {
    myMissShader = aShader;
  }

  uint RaytracingPipelineStateProperties::AddHitGroup(const char* aName, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader, RaytracingHitGroupType aType)
  {
    ASSERT(anIntersectionShader || anAnyHitShader || aClosestHitShader, "At least one hit shader needs to be non-null");
    ASSERT(eastl::find_if(myHitGroups.begin(), myHitGroups.end(), [aName](const HitGroup& aHitGroup) { return aHitGroup.myName == aName; }) == myHitGroups.end(), "Hit group named %s already added", aName);

    HitGroup& group = myHitGroups.push_back();
    group.myType = aType;
    group.myName = aName;
    group.myIntersectionShaderIdx = AddUniqueHitShaderGetIndex(anIntersectionShader);
    group.myAnyHitShaderIdx = AddUniqueHitShaderGetIndex(anAnyHitShader);
    group.myClosestHitShaderIdx = AddUniqueHitShaderGetIndex(aClosestHitShader);

    return (uint) myHitGroups.size() - 1;
  }

  uint64 RaytracingPipelineStateProperties::GetHash() const
  {
    MathUtil::Hasher hasher;
    for (const HitGroup& hit : myHitGroups)
    {
      hasher.Add(hit.myName);
      hasher.Add(hit.myIntersectionShaderIdx);
      hasher.Add(hit.myAnyHitShaderIdx);
      hasher.Add(hit.myClosestHitShaderIdx);
    }

    for (const HitShader& entry : myHitShaders)
    {
      hasher.Add(entry.myUniqueMainFunctionName);
      hasher.Add(entry.myShader->GetNativeBytecodeHash());
    }

    if (myRaygenShader)
      hasher.Add(myRaygenShader->GetNativeBytecodeHash());

    if (myMissShader)
      hasher.Add(myMissShader->GetNativeBytecodeHash());

    hasher.Add(myMaxPayloadSizeBytes);
    hasher.Add(myMaxAttributeSizeBytes);
    
    return hasher.GetHashValue();
  }

  uint RaytracingPipelineStateProperties::AddUniqueHitShaderGetIndex(const SharedPtr<Shader>& aShader)
  {
    if (!aShader)
      return UINT_MAX;

    HitShader* it = eastl::find_if(myHitShaders.begin(), myHitShaders.end(), [aShader](const HitShader& anEntry) { return anEntry.myShader == aShader; });
    if (it != myHitShaders.end())
      return it - myHitShaders.begin();
    
    const uint index = (uint)myHitShaders.size();
    HitShader& entry = myHitShaders.push_back();
    entry.myShader = aShader;
    entry.myUniqueMainFunctionName.sprintf("%s_%d", aShader->GetDescription().myMainFunction.c_str(), index);
    return index;
  }
}


