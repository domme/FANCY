#include "ModelComponent.h"
#include "Model.h"
#include "GeometryData.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  ModelComponent::ModelComponent()
  {

  }
//---------------------------------------------------------------------------//
  ModelComponent::~ModelComponent()
  {

  }
//---------------------------------------------------------------------------//
  void ModelComponent::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&m_pModel, "m_pModel");
  }
//---------------------------------------------------------------------------//
  void ModelComponent::update()
  {
    
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene