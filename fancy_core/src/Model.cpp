#include "Model.h"
#include "Serializer.h"
#include "SubModel.h"
#include "GraphicsWorld.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  Model::Model()
  {

  }
//---------------------------------------------------------------------------//
  Model::~Model()
  {

  }
//---------------------------------------------------------------------------//
  bool Model::operator==(const Model& anOther) const 
  {
    if (m_vSubModels.size() != anOther.m_vSubModels.size())
      return false;

    bool equal = true;
    for (uint i = 0u; equal && i < m_vSubModels.size(); ++i)
      equal &= m_vSubModels[i] == anOther.m_vSubModels[i];
    
    return equal;
  }
//---------------------------------------------------------------------------//
  bool Model::operator==(const ModelDesc& aDesc) const 
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  ModelDesc Model::GetDescription() const
  {
    ModelDesc desc;

    desc.mySubmodels.resize(m_vSubModels.size());
    for (uint i = 0u; i < m_vSubModels.size(); ++i)
      desc.mySubmodels[i] = m_vSubModels[i]->GetDescription();

    return desc;
  }
//---------------------------------------------------------------------------//
  void Model::SetFromDescription(const ModelDesc& aDesc, GraphicsWorld* aWorld)
  {
    if (GetDescription() == aDesc)
      return;

    m_vSubModels.resize(aDesc.mySubmodels.size());
    for (uint i = 0u; i < aDesc.mySubmodels.size(); ++i)
      m_vSubModels[i] = aWorld->CreateSubModel(aDesc.mySubmodels[i]);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

