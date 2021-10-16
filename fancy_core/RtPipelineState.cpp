#include "fancy_core_precompile.h"
#include "RtPipelineState.h"

#include "Shader.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace Fancy
{
  namespace
  {
    uint locAddUniqueShaderGetIndex(const SharedPtr<Shader>& aShader, const char* aType, eastl::vector<RtPipelineStateProperties::ShaderEntry>& someShaders)
    {
      if (!aShader)
        return UINT_MAX;

      RtPipelineStateProperties::ShaderEntry* it = 
        eastl::find_if(someShaders.begin(), someShaders.end(), [aShader](const RtPipelineStateProperties::ShaderEntry& anEntry) { return anEntry.myShader == aShader; });

      if (it != someShaders.end())
        return uint(it - someShaders.begin());

      const uint index = (uint)someShaders.size();
      RtPipelineStateProperties::ShaderEntry& entry = someShaders.push_back();
      entry.myShader = aShader;
      entry.myUniqueMainFunctionName = StringUtil::ToWideString(aShader->GetDescription().myMainFunction.c_str());

      eastl::wstring wType = StringUtil::ToWideString(aType);
      entry.myUniqueMainFunctionName.append_sprintf(L"_%ls_%d", wType.c_str(), index);
      return index;
    }

    SharedPtr<Shader> locLoadShader(const char* aPath, const char* aMainFunction, const char* someDefines, ShaderStage aShaderStage)
    {
      if (aPath == nullptr || aPath[0] == '\0')
        return nullptr;

      ShaderDesc desc;
      desc.myPath = aPath;
      desc.myMainFunction = aMainFunction;
      desc.myShaderStage = aShaderStage;
      StringUtil::Tokenize(someDefines, ",", desc.myDefines);
      return RenderCore::CreateShader(desc);
    }
  }

  uint RtPipelineStateProperties::AddRayGenShader(const char* aPath, const char* aMainFunction, const char* someDefines)
  {
    return AddRayGenShader(locLoadShader(aPath, aMainFunction, someDefines, SHADERSTAGE_RAYGEN));
  }

  uint RtPipelineStateProperties::AddRayGenShader(const SharedPtr<Shader>& aShader)
  {
    return locAddUniqueShaderGetIndex(aShader, "RayGen", myRaygenShaders);
  }

  uint RtPipelineStateProperties::AddMissShader(const char* aPath, const char* aMainFunction, const char* someDefines)
  {
    return AddMissShader(locLoadShader(aPath, aMainFunction, someDefines, SHADERSTAGE_MISS));
  }

  uint RtPipelineStateProperties::AddMissShader(const SharedPtr<Shader>& aShader)
  {
    return locAddUniqueShaderGetIndex(aShader, "Miss", myMissShaders);
  }

  uint RtPipelineStateProperties::AddHitGroup(const wchar_t* aName, RtHitGroupType aType, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader)
  {
    ASSERT(anIntersectionShader || anAnyHitShader || aClosestHitShader, "At least one hit shader needs to be non-null");
    ASSERT(eastl::find_if(myHitGroups.begin(), myHitGroups.end(), [aName](const HitGroup& aHitGroup) { return aHitGroup.myName == aName; }) == myHitGroups.end(), "Hit group named %s already added", aName);

    HitGroup& group = myHitGroups.push_back();
    group.myType = aType;
    group.myName = aName;
    group.myIntersectionShaderIdx = locAddUniqueShaderGetIndex(anIntersectionShader, "Intersection", myHitShaders);
    group.myAnyHitShaderIdx = locAddUniqueShaderGetIndex(anAnyHitShader, "AnyHit", myHitShaders);
    group.myClosestHitShaderIdx = locAddUniqueShaderGetIndex(aClosestHitShader, "ClosestHit", myHitShaders);

    return (uint) myHitGroups.size() - 1;
  }

  uint RtPipelineStateProperties::AddHitGroup(const wchar_t* aName, RtHitGroupType aType, 
    const char* anIntersectionPath, const char* anIntersectionMainFunction, 
    const char* anAnyHitPath, const char* anAnyHitMainFunction,
    const char* aClosestHitPath, const char* aClosestHitMainFunction, 
    const char* someDefines)
  {
    return AddHitGroup(aName, aType,
      locLoadShader(anIntersectionPath, anIntersectionMainFunction, someDefines, SHADERSTAGE_INTERSECTION),
      locLoadShader(anAnyHitPath, anAnyHitMainFunction, someDefines, SHADERSTAGE_ANYHIT),
      locLoadShader(aClosestHitPath, aClosestHitMainFunction, someDefines, SHADERSTAGE_CLOSEST_HIT));
  }

  uint64 RtPipelineStateProperties::GetHash() const
  {
    MathUtil::Hasher hasher;
    for (const HitGroup& hit : myHitGroups)
    {
      hasher.Add(hit.myName);
      hasher.Add(hit.myIntersectionShaderIdx);
      hasher.Add(hit.myAnyHitShaderIdx);
      hasher.Add(hit.myClosestHitShaderIdx);
    }

    for (const ShaderEntry& entry : myHitShaders)
    {
      hasher.Add(entry.myUniqueMainFunctionName);
      hasher.Add(entry.myShader->GetNativeBytecodeHash());
    }

    for (const ShaderEntry& entry : myMissShaders)
    {
      hasher.Add(entry.myUniqueMainFunctionName);
      hasher.Add(entry.myShader->GetNativeBytecodeHash());
    }

    for (const ShaderEntry& entry : myRaygenShaders)
    {
      hasher.Add(entry.myUniqueMainFunctionName);
      hasher.Add(entry.myShader->GetNativeBytecodeHash());
    }

    hasher.Add(myMaxPayloadSizeBytes);
    hasher.Add(myMaxAttributeSizeBytes);
    hasher.Add(myMaxRecursionDepth);
    hasher.Add((uint)myPipelineFlags);
        
    return hasher.GetHashValue();
  }

  bool RtPipelineState::HasShader(const Shader* aShader) const
  {
    if (!IsRaytracingStage(aShader->myProperties.myShaderStage))
      return false;

    if (aShader->myProperties.myShaderStage == SHADERSTAGE_RAYGEN)
    {
      for (const auto& shader : myProperties.myRaygenShaders)
        if (shader.myShader.get() == aShader)
          return true;
    }
    else if (aShader->myProperties.myShaderStage == SHADERSTAGE_MISS)
    {
      for (const auto& shader : myProperties.myMissShaders)
        if (shader.myShader.get() == aShader)
          return true;
    }

    for (const auto& shader : myProperties.myHitShaders)
      if (shader.myShader.get() == aShader)
        return true;

    return false;
  }

  RtShaderIdentifier RtPipelineState::GetRayGenShaderIdentifier(uint anIndex)
  {
    ASSERT((uint)myProperties.myRaygenShaders.size() > anIndex);
    RtShaderIdentifier record;
    record.myType = RtShaderIdentifierType::RT_SHADER_IDENTIFIER_TYPE_RAYGEN;
    GetShaderIdentifierDataInternal(anIndex, myProperties.myRaygenShaders[anIndex], record);
    return record;
  }

  RtShaderIdentifier RtPipelineState::GetMissShaderIdentifier(uint anIndex)
  {
    ASSERT((uint)myProperties.myMissShaders.size() > anIndex);
    RtShaderIdentifier record;
    record.myType = RtShaderIdentifierType::RT_SHADER_IDENTIFIER_TYPE_MISS;
    GetShaderIdentifierDataInternal(anIndex, myProperties.myMissShaders[anIndex], record);
    return record;
  }

  RtShaderIdentifier RtPipelineState::GetHitShaderIdentifier(uint anIndex)
  {
    ASSERT((uint)myProperties.myHitGroups.size() > anIndex);
    RtShaderIdentifier record;
    record.myType = RtShaderIdentifierType::RT_SHADER_IDENTIFIER_TYPE_HIT;
    GetShaderIdentifierDataInternal(anIndex, myProperties.myHitGroups[anIndex], record);
    return record;
  }
}


