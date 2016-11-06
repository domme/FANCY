#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"
#include "ShaderResourceInterface.h"

#if defined (RENDERER_DX12)

#include "GpuProgramPipelineDX12.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgram.h"
#include "GpuProgramPipeline.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuProgramPipelineDX12::GpuProgramPipelineDX12() 
    : myShaderHash(0u)
    , myResourceInterface(nullptr)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineDX12::~GpuProgramPipelineDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::NotifyChangedShaders(const std::vector<GpuProgramDX12*>& someChangedPrograms)
  {
    std::vector<GpuProgramPipeline*> pipelinesToUpdate;

    auto& pipelineCache = GpuProgramPipeline::getRegisterMap();

    for (auto it = pipelineCache.begin(); it != pipelineCache.end(); ++it)
    {
      GpuProgramPipeline* pipeline = it->second;

      bool hasShader = false;
      for (GpuProgram* program : pipeline->myGpuPrograms)
      {
        for (GpuProgramDX12* changedProgram : someChangedPrograms)
        {
          hasShader |= program == changedProgram;
        }
      }

      if (hasShader)
        pipelinesToUpdate.push_back(pipeline);
    }

    // TODO next: change function so that pipelines are changed right away (i.e. change the programs in the correct stages...)

    for (GpuProgramPipeline* pipeline : pipelinesToUpdate)
    {
      
    }
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineDX12::operator==(const GpuProgramPipelineDX12& anOther) const
  {
    return myShaderHash == anOther.myShaderHash;
  }
  //---------------------------------------------------------------------------//
  GpuProgramPipelineDesc GpuProgramPipelineDX12::GetDescription() const
  {
    GpuProgramPipelineDesc desc;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i];
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
  //---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::SetFromDescription(const GpuProgramPipelineDesc& aDesc)
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      myGpuPrograms[i] = GpuProgram::FindFromDesc(aDesc.myGpuPrograms[i]);
    }

    myResourceInterface = nullptr;
    if (GpuProgram* vertexShader = myGpuPrograms[(uint32) ShaderStage::VERTEX])
    {
      myResourceInterface = vertexShader->GetResourceInterface();
    }

    RecomputeHashFromShaders();
  }
//---------------------------------------------------------------------------//
  ID3D12RootSignature* GpuProgramPipelineDX12::GetRootSignature() const
  {
    return myResourceInterface->myRootSignature.Get();
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineDX12::operator==(const GpuProgramPipelineDesc& anOtherDesc) const
  {
    return GetDescription() == anOtherDesc;
  }

//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::RecomputeHashFromShaders()
  {
    myShaderHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      MathUtil::hash_combine(myShaderHash, reinterpret_cast<uint>(myGpuPrograms[i]));
    }
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serializeArray(myGpuPrograms, "myGpuPrograms");
  }
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12