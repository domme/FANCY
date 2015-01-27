#ifndef INCLUDE_MODEL_H
#define INCLUDE_MODEL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class SubModel;
//---------------------------------------------------------------------------//
  const uint32 kMaxNumSubModelsPerModel = 256;
//---------------------------------------------------------------------------//
  typedef FixedArray<SubModel*, kMaxNumSubModelsPerModel> SubModelList;
//---------------------------------------------------------------------------//
  /*@brief: A Model is a collection of several SubModels. Each SubModel is potentially rendered with a different material
  and a different mesh */ 
  class Model : public StaticManagedHeapObject<Model> 
  {
    public:
      Model();
      ~Model();

      const ObjectName& getName() const {return m_Name;}
      void setName(const ObjectName& clNewName) {m_Name = clNewName;}

      uint32 getNumSubModels() const {return m_vSubModels.size();}
    //---------------------------------------------------------------------------//
      void addSubModel(SubModel* _pSubModel);
    //---------------------------------------------------------------------------//
      SubModel* getSubModel(uint32 u32Index) {return m_vSubModels[u32Index];}
      const SubModel* getSubModel(uint32 u32Index) const {return m_vSubModels[u32Index];}
    //---------------------------------------------------------------------------//
      SubModelList& getSubModelList() {return m_vSubModels;}
      const SubModelList& getSubModelList() const {return m_vSubModels;}
    //---------------------------------------------------------------------------//
      void setSubModelList(const SubModelList& _vSubModels) 
      {ASSERT(m_vSubModels.empty()); m_vSubModels = _vSubModels; }
    //---------------------------------------------------------------------------//
    private:
      SubModelList m_vSubModels;
      ObjectName m_Name;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Model)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

#endif  // INCLUDE_MODEL_H