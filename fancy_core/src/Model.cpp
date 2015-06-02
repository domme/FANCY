#include "Model.h"
#include "Serializer.h"

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
  void Model::serialize(IO::Serializer& aSerializer)
  {
    aSerializer.beginType(getTypeName(), getName());
    aSerializer.serialize(_VAL(m_Name));
    aSerializer.serialize(_VAL(m_vSubModels));
    aSerializer.endType();
  }
//---------------------------------------------------------------------------//
  void Model::addSubModel(SubModel* _pSubModel)
  {
    for (uint32 i = 0u; i < m_vSubModels.size(); ++i)
    {
      if (m_vSubModels[i] == _pSubModel)
      {
        return;
      }
    }

    m_vSubModels.push_back(_pSubModel);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

