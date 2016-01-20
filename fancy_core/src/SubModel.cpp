#include "SubModel.h"
#include "Serializer.h"
#include "Mesh.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  bool SubModelDesc::operator==(const SubModelDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  uint64 SubModelDesc::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, myMaterial.GetHash());
    MathUtil::hash_combine(hash, myMesh.GetHash());
    return hash;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  SubModel::SubModel() :
    m_pMaterial(nullptr),
    m_pMesh(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  SubModel::~SubModel()
  {

  }
//---------------------------------------------------------------------------//
  bool SubModel::operator==(const SubModel& anOther) const
  {
    return GetDescription() == anOther.GetDescription();
  }
//---------------------------------------------------------------------------//
  bool SubModel::operator==(const SubModelDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  SubModelDesc SubModel::GetDescription() const
  {
    SubModelDesc desc;
    desc.myMaterial = m_pMaterial->GetDescription();
    desc.myMesh = m_pMesh->GetDescription();
    return desc;
  }
//---------------------------------------------------------------------------//
  void SubModel::SetFromDescription(const SubModelDesc& aDesc)
  {
  }
//---------------------------------------------------------------------------//
  void SubModel::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_pMaterial, "m_pMaterial");
    aSerializer->serialize(&m_pMesh, "m_pMesh");
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry

