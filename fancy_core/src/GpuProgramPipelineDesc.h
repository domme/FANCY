#pragma once

#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"
#include "MathUtil.h"
#include "DescriptionBase.h"
#include "GpuProgramDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//  
  struct GpuProgramPipelineDesc : public DescriptionBase
  {
    bool operator==(const GpuProgramPipelineDesc& anOther) const;
    uint64 GetHash() const override;

    ~GpuProgramPipelineDesc() override {}
    ObjectName GetTypeName() const override { return _N(GpuProgramPipeline); }
    void Serialize(IO::Serializer* aSerializer) override;
    bool IsEmpty() const override;

    GpuProgramDesc myGpuPrograms[(uint32)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramPipelineDesc::operator==(const GpuProgramPipelineDesc& anOther) const
  {
    bool equal = true;
    for (uint32 i = 0u; equal && i < (uint32)ShaderStage::NUM; ++i)
      equal &= myGpuPrograms[i] == anOther.myGpuPrograms[i];
    return equal;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramPipelineDesc::GetHash() const
  {
    uint64 hash = 0u;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      MathUtil::hash_combine(hash, myGpuPrograms[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  inline void GpuProgramPipelineDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serializeArray(myGpuPrograms, "myGpuPrograms");
  }
//---------------------------------------------------------------------------//
  inline bool GpuProgramPipelineDesc::IsEmpty() const
  {
    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      if (!myGpuPrograms[i].IsEmpty())
        return false;

    return true;
  }
//---------------------------------------------------------------------------//
} }

