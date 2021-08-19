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
      RtPsoBuilder(uint aNumShaders, uint aNumHitGroups);

      void AddShaderLibrarySubobject(Shader* aShader, const wchar_t* aMainFunctionRename = nullptr);
      void AddHitGroupSubobject(const RaytracingPipelineStateProperties::HitGroup& aHitGroup, const eastl::vector<RaytracingPipelineStateProperties::ShaderEntry>& someHitShaders);
      void AddShaderConfig(uint aMaxPayloadSizeBytes, uint aMaxAttributeSizeBytes);
      void AddGlobalRootSignature(ID3D12RootSignature* aRootSignature);
      void SetPipelineConfig(uint aMaxRecursionDepth, RaytracingPipelineFlags someFlags);
      void AddSubobject(D3D12_STATE_SUBOBJECT_TYPE aType, void* aSubobject);
      const wchar_t* AddWideString(const char* aString);
      const wchar_t* AddWideString(const eastl::string& aString);
      bool BuildRtPso(Microsoft::WRL::ComPtr<ID3D12StateObject>& aStateObjectOut);

      D3D12_RAYTRACING_PIPELINE_CONFIG1 myPipelineConfig = {UINT_MAX};
      eastl::vector<D3D12_STATE_SUBOBJECT> mySubObjects;
      eastl::vector<D3D12_DXIL_LIBRARY_DESC> myLibraryDescs;
      eastl::vector<D3D12_EXPORT_DESC> myExportDescs;
      eastl::vector<D3D12_HIT_GROUP_DESC> myHitGroups;
      eastl::vector<D3D12_RAYTRACING_SHADER_CONFIG> myShaderConfigs;
      eastl::vector<D3D12_GLOBAL_ROOT_SIGNATURE> myGlobalRootSigs;
      eastl::vector<eastl::wstring> myWideStrings;
    };

    RtPsoBuilder::RtPsoBuilder(uint aNumShaders, uint aNumHitGroups)
    {
      myLibraryDescs.reserve(aNumShaders);
      myExportDescs.reserve(aNumShaders);
      myHitGroups.reserve(aNumHitGroups);
      myWideStrings.reserve(aNumShaders);
    }

//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddShaderLibrarySubobject(Shader* aShader, const wchar_t* aMainFunctionRename)
    {
      ShaderDX12* shaderDx12 = (ShaderDX12*)aShader;

      D3D12_EXPORT_DESC& exportDesc = myExportDescs.push_back();
      exportDesc = {};
      if (aMainFunctionRename)
      {
        exportDesc.Name = aMainFunctionRename;
        exportDesc.ExportToRename = AddWideString(shaderDx12->myDesc.myMainFunction);
      }
      else
      {
        exportDesc.Name = AddWideString(shaderDx12->myDesc.myMainFunction);
      }

      ASSERT(myLibraryDescs.capacity() > myLibraryDescs.size(), "Vector-grow not allowed. Individual members are being referenced as pointers.");
      D3D12_DXIL_LIBRARY_DESC& libDesc = myLibraryDescs.push_back();
      libDesc.DXILLibrary = shaderDx12->getNativeByteCode();
      libDesc.NumExports = 1;
      libDesc.pExports = &exportDesc;

      AddSubobject(D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &libDesc);
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddHitGroupSubobject(const RaytracingPipelineStateProperties::HitGroup& aHitGroup, const eastl::vector<RaytracingPipelineStateProperties::ShaderEntry>& someHitShaders)
    {
      ASSERT(myHitGroups.capacity() > myHitGroups.size(), "Vector-grow not allowed. Individual members are being referenced as pointers.");
      D3D12_HIT_GROUP_DESC& hitGroupDesc = myHitGroups.push_back();
      hitGroupDesc = {};
      hitGroupDesc.Type = RenderCore_PlatformDX12::GetRaytracingHitGroupType(aHitGroup.myType);
      hitGroupDesc.HitGroupExport = aHitGroup.myName.c_str();

      if (aHitGroup.myIntersectionShaderIdx != UINT_MAX)
        hitGroupDesc.IntersectionShaderImport = someHitShaders[aHitGroup.myIntersectionShaderIdx].myUniqueMainFunctionName.c_str();
      if (aHitGroup.myAnyHitShaderIdx != UINT_MAX)
        hitGroupDesc.AnyHitShaderImport = someHitShaders[aHitGroup.myAnyHitShaderIdx].myUniqueMainFunctionName.c_str();
      if (aHitGroup.myClosestHitShaderIdx != UINT_MAX)
        hitGroupDesc.ClosestHitShaderImport = someHitShaders[aHitGroup.myClosestHitShaderIdx].myUniqueMainFunctionName.c_str();

      AddSubobject(D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroupDesc);
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddShaderConfig(uint aMaxPayloadSizeBytes, uint aMaxAttributeSizeBytes)
    {
      D3D12_RAYTRACING_SHADER_CONFIG& config = myShaderConfigs.push_back();
      config.MaxPayloadSizeInBytes = aMaxPayloadSizeBytes;
      config.MaxAttributeSizeInBytes = aMaxAttributeSizeBytes;

      AddSubobject(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &config);
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddGlobalRootSignature(ID3D12RootSignature* aRootSignature)
    {
      D3D12_GLOBAL_ROOT_SIGNATURE& rootSig = myGlobalRootSigs.push_back();
      rootSig.pGlobalRootSignature = aRootSignature;

      AddSubobject(D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &rootSig);
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::SetPipelineConfig(uint aMaxRecursionDepth, RaytracingPipelineFlags someFlags)
    {
      ASSERT(myPipelineConfig.MaxTraceRecursionDepth == UINT_MAX, "Pipeline config already added");

      myPipelineConfig.MaxTraceRecursionDepth = aMaxRecursionDepth;
      myPipelineConfig.Flags = RenderCore_PlatformDX12::GetRaytracingPipelineFlags(someFlags);

      AddSubobject(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1, &myPipelineConfig);
    }
//---------------------------------------------------------------------------//
    void RtPsoBuilder::AddSubobject(D3D12_STATE_SUBOBJECT_TYPE aType, void* aSubobject)
    {
      D3D12_STATE_SUBOBJECT& subobject = mySubObjects.push_back();
      subobject.Type = aType;
      subobject.pDesc = aSubobject;
    }
//---------------------------------------------------------------------------//
    const wchar_t* RtPsoBuilder::AddWideString(const char* aString)
    {
      ASSERT(myWideStrings.capacity() > myWideStrings.size(), "Vector-grow not allowed. Individual members are being referenced as pointers.");
      myWideStrings.push_back(StringUtil::ToWideString(aString));
      return myWideStrings.back().c_str();
    }
//---------------------------------------------------------------------------//
    const wchar_t* RtPsoBuilder::AddWideString(const eastl::string& aString)
    {
      return AddWideString(aString.c_str());
    }
//---------------------------------------------------------------------------//
    bool RtPsoBuilder::BuildRtPso(Microsoft::WRL::ComPtr<ID3D12StateObject>& aStateObjectOut)
    {
      D3D12_STATE_OBJECT_DESC aDesc = {};
      aDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
      aDesc.NumSubobjects = (uint) mySubObjects.size();
      aDesc.pSubobjects = mySubObjects.data();

      HRESULT result = RenderCore::GetPlatformDX12()->GetDevice()->CreateStateObject(&aDesc, IID_PPV_ARGS(&aStateObjectOut));
      return result == S_OK;
    }
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RaytracingPipelineStateDX12::RaytracingPipelineStateDX12(const RaytracingPipelineStateProperties& someProps)
    : RaytracingPipelineState(someProps)
  {
    Recompile();
  }
//---------------------------------------------------------------------------//
  bool RaytracingPipelineStateDX12::Recompile()
  {
    ASSERT(!myProperties.myRaygenShaders.empty(), "Raygen shader is required to build an RT PSO");

    const uint numShaders = static_cast<uint>(myProperties.myHitShaders.size() + myProperties.myRaygenShaders.size() + myProperties.myMissShaders.size());
    const uint numHitGroups = static_cast<uint>(myProperties.myHitGroups.size());
    RtPsoBuilder builder(numShaders, numHitGroups);

    for (const RaytracingPipelineStateProperties::ShaderEntry& shader : myProperties.myRaygenShaders)
      builder.AddShaderLibrarySubobject(shader.myShader.get(), shader.myUniqueMainFunctionName.c_str());

    for (const RaytracingPipelineStateProperties::ShaderEntry& shader : myProperties.myMissShaders)
      builder.AddShaderLibrarySubobject(shader.myShader.get(), shader.myUniqueMainFunctionName.c_str());

    for (const RaytracingPipelineStateProperties::ShaderEntry& it : myProperties.myHitShaders)
      builder.AddShaderLibrarySubobject(it.myShader.get(), it.myUniqueMainFunctionName.c_str());

    for (const RaytracingPipelineStateProperties::HitGroup& hitGroup : myProperties.myHitGroups)
      builder.AddHitGroupSubobject(hitGroup, myProperties.myHitShaders);

    builder.AddShaderConfig(myProperties.myMaxPayloadSizeBytes, myProperties.myMaxAttributeSizeBytes);

    RenderCore_PlatformDX12* platformDx12 = RenderCore::GetPlatformDX12();
    builder.AddGlobalRootSignature(platformDx12->GetRootSignature()->GetRootSignature());

    builder.SetPipelineConfig(myProperties.myMaxRecursionDepth, myProperties.myPipelineFlags);

    Microsoft::WRL::ComPtr<ID3D12StateObject> rtPso;
    if (builder.BuildRtPso(rtPso))
    {
      ASSERT_HRESULT(rtPso.As(&myRtPsoProperties));

      myStateObject = rtPso;
      return true;
    }

    LOG_ERROR("Failed building RTPSO");
    return false;
  }
//---------------------------------------------------------------------------//
  void RaytracingPipelineStateDX12::GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aShaderEntry, RaytracingShaderRecord& someDataOut)
  {
    void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(aShaderEntry.myName.c_str());
    ASSERT(shaderIdentifier);

    someDataOut.myData.resize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    memcpy(someDataOut.myData.data(), shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
  }
//---------------------------------------------------------------------------//
  void RaytracingPipelineStateDX12::GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry, RaytracingShaderRecord& someDataOut)
  {
    void* shaderIdentifier = myRtPsoProperties->GetShaderIdentifier(aShaderEntry.myUniqueMainFunctionName.c_str());
    ASSERT(shaderIdentifier);

    someDataOut.myData.resize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    memcpy(someDataOut.myData.data(), shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
  }
//---------------------------------------------------------------------------//
}
