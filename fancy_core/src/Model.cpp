#include "Model.h"
#include "Serializer.h"
#include "SubModel.h"

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
  void Model::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_vSubModels, "m_vSubModels");
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

