#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramPipelineDesc.h"

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
  
    uint64 GetHash() const { return GetDescription().GetHash(); }
    uint64 GetShaderByteCodeHash() const { return myShaderByteCodeHash; }

    GpuProgramPipelineDX12();
    ~GpuProgramPipelineDX12();
    bool operator==(const GpuProgramPipelineDX12& anOther) const;
    bool operator==(const GpuProgramPipelineDesc& anOtherDesc) const;

    GpuProgramPipelineDesc GetDescription() const;
    void SetFromShaders(const std::array<SharedPtr<GpuProgram>, (uint32) ShaderStage::NUM>& someShaders);
    ID3D12RootSignature* GetRootSignature() const;
    
    void UpdateResourceInterface();
    void UpdateShaderByteCodeHash();

    SharedPtr<GpuProgram> myGpuPrograms[(uint32)ShaderStage::NUM];

    uint myShaderByteCodeHash;  /// Can be used as "deep" comparison that is also affected when shaders are recompiled
    const ShaderResourceInterface* myResourceInterface;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12
