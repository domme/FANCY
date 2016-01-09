#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

#include "Serializable.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuProgramPipelineDX12
  {
  public:
    SERIALIZABLE(GpuProgramPipelineDX12);

    void serialize(IO::Serializer* aSerializer);
    static ObjectName getTypeName() { return _N(GpuProgramPipeline); }
    ObjectName getName() const { return ObjectName(myShaderHash); }

    GpuProgramPipelineDX12();
    ~GpuProgramPipelineDX12();
    bool operator==(const GpuProgramPipelineDX12& anOther);
    
    void RecomputeHashFromShaders();

    GpuProgram* myGpuPrograms[(uint32)ShaderStage::NUM];

    uint myShaderHash;  /// Used to quickly compare two pipelines
    ComPtr<ID3D12RootSignature> myRootSignature;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12
