#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramDesc.h"

#if defined (RENDERER_DX12)

#include "Serializable.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
  class ShaderResourceInterface;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuProgramPipelineDX12
  {
  public:
    SERIALIZABLE(GpuProgramPipelineDX12);

    static void NotifyChangedShaders(const std::vector<GpuProgramDX12*>& someChangedPrograms);

    void serialize(IO::Serializer* aSerializer);
    static ObjectName getTypeName() { return _N(GpuProgramPipeline); }
    uint64 GetHash() const { return GetDescription().GetHash(); }

    GpuProgramPipelineDX12();
    ~GpuProgramPipelineDX12();
    bool operator==(const GpuProgramPipelineDX12& anOther) const;
    bool operator==(const GpuProgramPipelineDesc& anOtherDesc) const;

    GpuProgramPipelineDesc GetDescription() const;
    void SetFromDescription(const GpuProgramPipelineDesc& aDesc);
    ID3D12RootSignature* GetRootSignature() const;
    
    void RecomputeHashFromShaders();

    GpuProgram* myGpuPrograms[(uint32)ShaderStage::NUM];

    uint myShaderHash;  /// Used to quickly compare two pipelines
    const ShaderResourceInterface* myResourceInterface;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12
