#include "SubModel.h"
#include "Serializer.h"
#include "Mesh.h"
#include "Renderer.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  bool SubModelDesc::operator==(const SubModelDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  void SubModelDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myMaterial, "myMaterial");
    aSerializer->Serialize(&myMesh, "myMesh");
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
    m_pMaterial(nullptr)
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
  void SubModel::SetFromDescription(const SubModelDesc& aDesc, GraphicsWorld* aWorld)
  {
    m_pMaterial = Rendering::Material::FindFromDesc(aDesc.myMaterial);
    m_pMesh = Rendering::RenderCore::GetMesh(aDesc.myMesh.myVertexAndIndexHash);
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry

