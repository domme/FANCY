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
    aSerializer.serialize(_VAL(m_Name));
    aSerializer.serialize(_VAL(m_pMaterial));
    aSerializer.serialize(_VAL(m_pMesh));
  }
//---------------------------------------------------------------------------//
} }   // end of namespace Fancy::Geometry
