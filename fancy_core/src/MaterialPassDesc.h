#pragma once

#include "BlendState.h"
#include "DepthStencilState.h"
#include "GpuProgramDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialPassDesc
  {
    MaterialPassDesc();
    bool operator==(const MaterialPassDesc& anOther) const;
    uint64 GetHash() const;

    uint32 m_eFillMode;
    uint32 m_eCullMode;
    uint32 m_eWindingOrder;
    BlendStateDesc m_BlendStateDesc;
    DepthStencilStateDesc m_DepthStencilStateDesc;
    GpuProgramPipelineDesc m_GpuProgramPipelineDesc;
  };
//---------------------------------------------------------------------------//
} }