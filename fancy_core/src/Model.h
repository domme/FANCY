#ifndef INCLUDE_MODEL_H
#define INCLUDE_MODEL_H

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"

namespace FANCY { namespace Core { namespace Geometry {
//---------------------------------------------------------------------------//
  class SubModel;
//---------------------------------------------------------------------------//
  typedef FixedArray<SubModel*, 512> SubModelList;
//---------------------------------------------------------------------------//
  class Model 
  {
    public:
      Model();
      ~Model();

      const ObjectName& getName() {return m_Name;}
      void setName(const ObjectName& clNewName) {m_Name = clNewName;}

      uint32 getNumSubModels() const {return m_vSubModels.size();}
    //---------------------------------------------------------------------------//
      SubModel* getSubModel(uint32 u32Index) {return m_vSubModels[u32Index];}
      const SubModel* getSubModel(uint32 u32Index) const {return m_vSubModels[u32Index];}
    //---------------------------------------------------------------------------//
      SubModelList& getSubModelList() {return m_vSubModels;}
      const SubModelList& getSubModelList() const {return m_vSubModels;}
    //---------------------------------------------------------------------------//
    private:
      SubModelList m_vSubModels;
      ObjectName m_Name;
  };
//---------------------------------------------------------------------------//
} } }  // end of namespace FANCY::Core::Geometry

#endif  // INCLUDE_MODEL_H