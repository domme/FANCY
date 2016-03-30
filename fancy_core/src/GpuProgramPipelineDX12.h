#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramDesc.h"

#if defined (RENDERER_DX12)

#include "Serializable.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RootSignatureDX12;
//---------------------------------------------------------------------------//
  class GpuProgramPipelineDX12
  {
  public:
    SERIALIZABLE(GpuProgramPipelineDX12);

    void serialize(IO::Serializer* aSerializer);
    static ObjectName getTypeName() { return _N(GpuProgramPipeline); }
    uint64 GetHash() const { return GetDescription().GetHash(); }

    GpuProgramPipelineDX12();
    ~GpuProgramPipelineDX12();
    bool operator==(const GpuProgramPipelineDX12& anOther) const;
    bool operator==(const GpuProgramPipelineDesc& anOtherDesc) const;

    GpuProgramPipelineDesc GetDescription() const;
    void SetFromDescription(const GpuProgramPipelineDesc& aDesc);
    ID3D12RootSignature* GetRootSignatureNative() const;
    
    void RecomputeHashFromShaders();

    GpuProgram* myGpuPrograms[(uint32)ShaderStage::NUM];

    uint myShaderHash;  /// Used to quickly compare two pipelines
    const RootSignatureDX12* myRootSignature;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12
