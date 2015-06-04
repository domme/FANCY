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
    aSerializer.serialize(_VAL(m_Name));
    
    uint32 num = aSerializer.beginArray("m_vSubModels", m_vSubModels.size());
    m_vSubModels.resize(num);
    for (uint32 i = 0u; i < num; ++i)
    {
      aSerializer.serialize(m_vSubModels[i]);
    }
    aSerializer.endArray();
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

