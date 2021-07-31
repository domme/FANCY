#include "fancy_core_precompile.h"
#include "RaytracingPipelineStateDX12.h"

#include "RenderCore_PlatformDX12.h"
#include "ShaderDX12.h"

namespace Fancy
{
  namespace
  {
//---------------------------------------------------------------------------//
    struct RtPsoBuilder
    {
      RtPsoBuilder();
      void AddShaderLibrarySubobject(Shader* aShader, const char* aMainFunctionRename = nullptr);
      void AddHitGroupSubobject(const RaytracingPipelineStateProperties::HitGroup& aHitGroup, const eastl::vector<RaytracingPipelineStateProperties::HitShader>& someHitShaders);
      void AddShaderConfig(uint aMaxPayloadSizeBytes, uint aMaxAttributeSizeBytes);
      const wchar_t* AddWideString(const char* aString);
      const wchar_t* AddWideString(const eastl::string& aString);

      D3D12_STATE_OBJECT_DESC myDesc;
      eastl::vector<D3D12_STATE_SUBOBJECT> mySubObjects;
      eastl::vector<D3D12_DXIL_LIBRARY_DESC> myLibraryDescs;
      eastl::vector<D3D12_EXPORT_DESC> myExportDescs;
      eastl::vector<D3D12_HIT_GROUP_DESC> myHitGroups;
      eastl::vector<D3D12_RAYTRACING_SHADER_CONFIG> myShaderConfigs;
      eastl::vector<eastl::wstring> myWideStrings;
    };
//---------------------------------------------------------------------------//
    RtPsoBuilder::RtPsoBuilder()
    : myDesc{}
    {
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddShaderLibrarySubobject(Shader* aShader, const char* aMainFunctionRename)
    {
      ShaderDX12* shaderDx12 = (ShaderDX12*)aShader;

      D3D12_EXPORT_DESC& exportDesc = myExportDescs.push_back();
      exportDesc = {};
      if (aMainFunctionRename)
      {
        exportDesc.Name = AddWideString(aMainFunctionRename);
        exportDesc.ExportToRename = AddWideString(shaderDx12->myDesc.myMainFunction);
      }
      else
      {
        exportDesc.Name = AddWideString(shaderDx12->myDesc.myMainFunction);
      }

      D3D12_DXIL_LIBRARY_DESC& libDesc = myLibraryDescs.push_back();
      libDesc.DXILLibrary = shaderDx12->getNativeByteCode();
      libDesc.NumExports = 1;
      libDesc.pExports = &exportDesc;

      D3D12_STATE_SUBOBJECT& subobject = mySubObjects.push_back();
      subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
      subobject.pDesc = &libDesc;
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddHitGroupSubobject(const RaytracingPipelineStateProperties::HitGroup& aHitGroup, const eastl::vector<RaytracingPipelineStateProperties::HitShader>& someHitShaders)
    {
      D3D12_HIT_GROUP_DESC& hitGroupDesc = myHitGroups.push_back();
      hitGroupDesc = {};
      hitGroupDesc.Type = RenderCore_PlatformDX12::GetRaytracingHitGroupType(aHitGroup.myType);
      hitGroupDesc.HitGroupExport = AddWideString(aHitGroup.myName);

      if (aHitGroup.myIntersectionShaderIdx != UINT_MAX)
        hitGroupDesc.IntersectionShaderImport = AddWideString(someHitShaders[aHitGroup.myIntersectionShaderIdx].myUniqueMainFunctionName);
      if (aHitGroup.myAnyHitShaderIdx != UINT_MAX)
        hitGroupDesc.AnyHitShaderImport = AddWideString(someHitShaders[aHitGroup.myAnyHitShaderIdx].myUniqueMainFunctionName);
      if (aHitGroup.myClosestHitShaderIdx != UINT_MAX)
        hitGroupDesc.ClosestHitShaderImport = AddWideString(someHitShaders[aHitGroup.myClosestHitShaderIdx].myUniqueMainFunctionName);

      D3D12_STATE_SUBOBJECT& subobject = mySubObjects.push_back();
      subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
      subobject.pDesc = &hitGroupDesc;
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddShaderConfig(uint aMaxPayloadSizeBytes, uint aMaxAttributeSizeBytes)
    {
      D3D12_RAYTRACING_SHADER_CONFIG& config = myShaderConfigs.push_back();
      config.MaxPayloadSizeInBytes = aMaxPayloadSizeBytes;
      config.MaxAttributeSizeInBytes = aMaxAttributeSizeBytes;

      D3D12_STATE_SUBOBJECT& subobject = mySubObjects.push_back();
      subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
      subobject.pDesc = &config;
    }
//---------------------------------------------------------------------------//
    const wchar_t* RtPsoBuilder::AddWideString(const char* aString)
    {
      myWideStrings.push_back(StringUtil::ToWideString(aString));
      return myWideStrings.back().c_str();
    }
//---------------------------------------------------------------------------//
    const wchar_t* RtPsoBuilder::AddWideString(const eastl::string& aString)
    {
      return AddWideString(aString.c_str());
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RaytracingPipelineStateDX12::RaytracingPipelineStateDX12(const RaytracingPipelineStateProperties& someProps)
    : RaytracingPipelineState(someProps)
  {
    Recompile();
  }

  bool RaytracingPipelineStateDX12::Recompile()
  {
    RtPsoBuilder builder;
    ASSERT(myProperties.myRaygenShader.get(), "Raygen shader is required to build an RT PSO");
    builder.AddShaderLibrarySubobject(myProperties.myRaygenShader.get());

    if (myProperties.myMissShader)
      builder.AddShaderLibrarySubobject(myProperties.myMissShader.get());

    for (const RaytracingPipelineStateProperties::HitShader& it : myProperties.myHitShaders)
      builder.AddShaderLibrarySubobject(it.myShader.get(), it.myUniqueMainFunctionName.c_str());

    for (const RaytracingPipelineStateProperties::HitGroup& hitGroup : myProperties.myHitGroups)
      builder.AddHitGroupSubobject(hitGroup, myProperties.myHitShaders);

    builder.AddShaderConfig(myProperties.myMaxPayloadSizeBytes, myProperties.myMaxAttributeSizeBytes);



  }

//---------------------------------------------------------------------------//
}



