#include "fancy_core_precompile.h"
#include "RaytracingPipelineState.h"

#include "Shader.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace Fancy
{
  namespace
  {
    uint locAddUniqueShaderGetIndex(const SharedPtr<Shader>& aShader, const char* aType, eastl::vector<RaytracingPipelineStateProperties::ShaderEntry>& someShaders)
    {
      if (!aShader)
        return UINT_MAX;

      RaytracingPipelineStateProperties::ShaderEntry* it = 
        eastl::find_if(someShaders.begin(), someShaders.end(), [aShader](const RaytracingPipelineStateProperties::ShaderEntry& anEntry) { return anEntry.myShader == aShader; });

      if (it != someShaders.end())
        return uint(it - someShaders.begin());

      const uint index = (uint)someShaders.size();
      RaytracingPipelineStateProperties::ShaderEntry& entry = someShaders.push_back();
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

  uint RaytracingPipelineStateProperties::AddRayGenShader(const char* aPath, const char* aMainFunction, const char* someDefines)
  {
    return AddRayGenShader(locLoadShader(aPath, aMainFunction, someDefines, SHADERSTAGE_RAYGEN));
  }

  uint RaytracingPipelineStateProperties::AddRayGenShader(const SharedPtr<Shader>& aShader)
  {
    return locAddUniqueShaderGetIndex(aShader, "RayGen", myRaygenShaders);
  }

  uint RaytracingPipelineStateProperties::AddMissShader(const char* aPath, const char* aMainFunction, const char* someDefines)
  {
    return AddMissShader(locLoadShader(aPath, aMainFunction, someDefines, SHADERSTAGE_MISS));
  }

  uint RaytracingPipelineStateProperties::AddMissShader(const SharedPtr<Shader>& aShader)
  {
    return locAddUniqueShaderGetIndex(aShader, "Miss", myMissShaders);
  }

  uint RaytracingPipelineStateProperties::AddHitGroup(const wchar_t* aName, RaytracingHitGroupType aType, const SharedPtr<Shader>& anIntersectionShader, const SharedPtr<Shader>& anAnyHitShader, const SharedPtr<Shader>& aClosestHitShader)
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

  uint RaytracingPipelineStateProperties::AddHitGroup(const wchar_t* aName, RaytracingHitGroupType aType, 
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

  RaytracingShaderRecord RaytracingPipelineState::GetRayGenShaderRecord(uint anIndex)
  {
    ASSERT((uint)myProperties.myRaygenShaders.size() > anIndex);
    RaytracingShaderRecord record;
    record.myType = RaytracingShaderRecordType::RT_SHADER_RECORD_TYPE_RAYGEN;
    GetShaderRecordDataInternal(anIndex, myProperties.myRaygenShaders[anIndex], record);
    return record;
  }

  RaytracingShaderRecord RaytracingPipelineState::GetMissShaderRecord(uint anIndex)
  {
    ASSERT((uint)myProperties.myMissShaders.size() > anIndex);
    RaytracingShaderRecord record;
    record.myType = RaytracingShaderRecordType::RT_SHADER_RECORD_TYPE_MISS;
    GetShaderRecordDataInternal(anIndex, myProperties.myMissShaders[anIndex], record);
    return record;
  }

  RaytracingShaderRecord RaytracingPipelineState::GetHitShaderRecord(uint anIndex)
  {
    ASSERT((uint)myProperties.myHitGroups.size() > anIndex);
    RaytracingShaderRecord record;
    record.myType = RaytracingShaderRecordType::RT_SHADER_RECORD_TYPE_HIT;
    GetShaderRecordDataInternal(anIndex, myProperties.myHitGroups[anIndex], record);
    return record;
  }
}


