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
    : myShaderByteCodeHash(0u)
    , myResourceInterface(nullptr)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineDX12::~GpuProgramPipelineDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineDX12::operator==(const GpuProgramPipelineDesc& anOtherDesc) const
  {
    return GetDescription() == anOtherDesc;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineDX12::operator==(const GpuProgramPipelineDX12& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
  //---------------------------------------------------------------------------//
  GpuProgramPipelineDesc GpuProgramPipelineDX12::GetDescription() const
  {
    GpuProgramPipelineDesc desc;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
    {
      const GpuProgram* pProgram = myGpuPrograms[i].get();
      if (pProgram)
        desc.myGpuPrograms[i] = pProgram->GetDescription();
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::SetFromShaders(const std::array<SharedPtr<GpuProgram>, (uint32)ShaderStage::NUM>& someShaders)
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      myGpuPrograms[i] = someShaders[i];
        
    UpdateResourceInterface();
    UpdateShaderByteCodeHash();
  }
//---------------------------------------------------------------------------//
  ID3D12RootSignature* GpuProgramPipelineDX12::GetRootSignature() const
  {
    return myResourceInterface->myRootSignature.Get();
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::UpdateResourceInterface()
  {
    myResourceInterface = nullptr;
    if (GpuProgram* vertexShader = myGpuPrograms[(uint32)ShaderStage::VERTEX].get())
    {
      myResourceInterface = vertexShader->GetResourceInterface();
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramPipelineDX12::UpdateShaderByteCodeHash()
  {
    myShaderByteCodeHash = 0u;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      GpuProgram* shader = myGpuPrograms[i].get();
      MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader));
      if (shader != nullptr)
        MathUtil::hash_combine(myShaderByteCodeHash, reinterpret_cast<uint64>(shader->getNativeData().Get()));
    }
  }
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12