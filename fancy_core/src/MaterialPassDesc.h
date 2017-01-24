#pragma once

#include "BlendState.h"
#include "DepthStencilState.h"
#include "GpuProgramPipelineDesc.h"
#include "DescriptionBase.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialPassDesc : public DescriptionBase
  {
    MaterialPassDesc();
    ~MaterialPassDesc() override {}

    bool operator==(const MaterialPassDesc& anOther) const;
    uint64 GetHash() const override;
    ObjectName GetTypeName() const override { return _N(MaterialPass); }
    void Serialize(IO::Serializer* aSerializer) override;

    uint32 m_eFillMode;
    uint32 m_eCullMode;
    uint32 m_eWindingOrder;
    BlendStateDesc m_BlendStateDesc;
    DepthStencilStateDesc m_DepthStencilStateDesc;
    GpuProgramPipelineDesc m_GpuProgramPipelineDesc;
  };
//---------------------------------------------------------------------------//
} }