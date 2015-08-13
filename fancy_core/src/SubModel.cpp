#include "SubModel.h"
#include "Serializer.h"
#include "Mesh.h"

namespace Fancy { namespace Geometry { 
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
  void SubModel::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_pMaterial, "m_pMaterial");
    aSerializer->serialize(&m_pMesh, "m_pMesh");
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry

