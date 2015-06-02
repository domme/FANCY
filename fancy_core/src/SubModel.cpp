#include "SubModel.h"
#include "Serializer.h"

namespace Fancy { namespace Geometry { 
//---------------------------------------------------------------------------//
  SubModel::SubModel() :
    m_pMesh(nullptr),
    m_pMaterial(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  SubModel::~SubModel()
  {

  }
//---------------------------------------------------------------------------//
  void SubModel::serialize(IO::Serializer& aSerializer)
  {
    aSerializer.beginType(getTypeName(), getName());

    aSerializer & m_Name;
    aSerializer & m_pMaterial;
    aSerializer & m_pMesh;
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry
