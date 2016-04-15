#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"
#include "ShaderResourceInterface.h"

#if defined (RENDERER_DX12)

#include "GpuProgramPipelineDX12.h"
#include "ShaderResourceInterfaceDX12.h"
#include "GpuProgram.h"

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
  void GpuProgramPipelineDX12::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serializeArray(myGpuPrograms, "myGpuPrograms");
  }
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12