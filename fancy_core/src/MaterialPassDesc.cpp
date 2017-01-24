#include "MaterialPassDesc.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  MaterialPassDesc::MaterialPassDesc()
    : m_eFillMode(0u)
    , m_eCullMode(0u)
    , m_eWindingOrder(0u)
  {

  }
//---------------------------------------------------------------------------//
  bool MaterialPassDesc::operator==(const MaterialPassDesc& anOther) const
  {
    return m_eFillMode == anOther.m_eFillMode &&
      m_eCullMode == anOther.m_eCullMode &&
      m_eWindingOrder == anOther.m_eWindingOrder &&
      m_BlendStateDesc == anOther.m_BlendStateDesc &&
      m_DepthStencilStateDesc == anOther.m_DepthStencilStateDesc &&
      m_GpuProgramPipelineDesc == anOther.m_GpuProgramPipelineDesc;
  }
//---------------------------------------------------------------------------//
  uint64 MaterialPassDesc::GetHash() const
  {
    uint64 hash;

    MathUtil::hash_combine(hash, m_eFillMode);
    MathUtil::hash_combine(hash, m_eCullMode);
    MathUtil::hash_combine(hash, m_eWindingOrder);
    MathUtil::hash_combine(hash, m_BlendStateDesc.GetHash());
    MathUtil::hash_combine(hash, m_DepthStencilStateDesc.GetHash());
    MathUtil::hash_combine(hash, m_GpuProgramPipelineDesc.GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  void MaterialPassDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&m_eFillMode, "m_eFillMode");
    aSerializer->Serialize(&m_eCullMode, "m_eCullMode");
    aSerializer->Serialize(&m_eWindingOrder, "m_eWindingOrder");
    aSerializer->Serialize(&m_BlendStateDesc, "m_BlendStateDesc");
    aSerializer->Serialize(&m_DepthStencilStateDesc, "m_DepthStencilStateDesc");
    aSerializer->Serialize(&m_GpuProgramPipelineDesc, "m_GpuProgramPipelineDesc");
  }
//---------------------------------------------------------------------------//
} }