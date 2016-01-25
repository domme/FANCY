#include "StdAfx.h"
#include "GpuProgramCompilerDX12.h"

#if defined (RENDERER_DX12)

#include "GpuProgramDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuProgramDX12::GpuProgramDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDX12::~GpuProgramDX12()
  {
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::Destroy()
  {
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::operator==(const GpuProgramDX12& anOther) const
  {
    return GetDescription() == anOther.GetDescription();
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::operator==(const GpuProgramDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::serialize(IO::Serializer* aSerializer)
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgramDX12::GetDescription() const
  {
    GpuProgramDesc desc;
    desc.myShaderPath = mySourcePath;
    desc.myShaderStage = static_cast<uint32>(myStage);
    desc.myPermutation = myPermutation;
    return desc;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramDX12::SetFromDescription(const GpuProgramDesc& aDesc)
  {
    GpuProgramCompilerOutputDX12 output;
    const bool success = GpuProgramCompilerDX12::Compile(aDesc, &output);

    if (success)
    {
      mySourcePath = aDesc.myShaderPath;
      myPermutation = aDesc.myPermutation;
      myStage = static_cast<ShaderStage>(aDesc.myShaderStage);

      Destroy();
      SetFromCompilerOutput(output);
    }

    return success;
  }
//---------------------------------------------------------------------------//
  void GpuProgramDX12::SetFromCompilerOutput(const GpuProgramCompilerOutputDX12& aCompilerOutput)
  {

  }
//---------------------------------------------------------------------------//
} } }

#endif
