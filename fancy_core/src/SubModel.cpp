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
    aSerializer.serialize(_VAL(m_Name));
    aSerializer.serialize(_VAL(m_pMaterial));
    aSerializer.serialize(_VAL(m_pMesh));
    aSerializer.endType();
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry
