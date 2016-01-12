#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MathUtil.h"
#include "Serializer.h"

#if defined (RENDERER_DX12)

#include "GpuProgramPipelineDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuProgramPipelineDX12::GpuProgramPipelineDX12() :
    myShaderHash(0u)
  {
    memset(myGpuPrograms, 0u, sizeof(myGpuPrograms));
  }
//---------------------------------------------------------------------------//
  GpuProgramPipelineDX12::~GpuProgramPipelineDX12()
  {
    
  }
//---------------------------------------------------------------------------//
  bool GpuProgramPipelineDX12::operator==(const GpuProgramPipelineDX12& anOther)
  {
    return myShaderHash == anOther.myShaderHash;
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