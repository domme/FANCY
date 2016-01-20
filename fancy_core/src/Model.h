#pragma once

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"
#include "Serializable.h"
#include "ModelDesc.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class SubModel;
//---------------------------------------------------------------------------//
  /*@brief: A Model is a collection of several SubModels. Each SubModel is potentially rendered with a different material
  and a different mesh */ 
  class Model : public StaticManagedHeapObject<Model>
  {
    public:
      SERIALIZABLE(Model)

      Model();
      ~Model();
      bool operator==(const Model& anOther) const;
      bool operator==(const ModelDesc& aDesc) const;

      ModelDesc GetDescription() const;
      void SetFromDescription(const ModelDesc& aDesc);

      static ObjectName getTypeName() { return _N(Model); }
      void serialize(IO::Serializer* aSerializer);
      
      const ObjectName& getName() const {return m_Name;}
      void setName(const ObjectName& clNewName) {m_Name = clNewName;}

      uint32 getNumSubModels() const {return m_vSubModels.size();}
    //---------------------------------------------------------------------------//
      void addSubModel(SubModel* _pSubModel);
    //---------------------------------------------------------------------------//
      SubModel* getSubModel(uint32 u32Index) {return m_vSubModels[u32Index];}
      const SubModel* getSubModel(uint32 u32Index) const {return m_vSubModels[u32Index];}
    //---------------------------------------------------------------------------//
      std::vector<SubModel*>& getSubModelList() {return m_vSubModels;}
      const std::vector<SubModel*>& getSubModelList() const {return m_vSubModels;}
    //---------------------------------------------------------------------------//
      void setSubModelList(const std::vector<SubModel*>& _vSubModels)
      {ASSERT(m_vSubModels.empty()); m_vSubModels = _vSubModels; }
    //---------------------------------------------------------------------------//
    private:
      ObjectName m_Name;
      std::vector<SubModel*> m_vSubModels;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Model)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

