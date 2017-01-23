#pragma once

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"
#include "Serializable.h"
#include "ModelDesc.h"

namespace Fancy {
  class GraphicsWorld;
}

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
      void SetFromDescription(const ModelDesc& aDesc, GraphicsWorld* aWorld);

      static ObjectName getTypeName() { return _N(Model); }
      uint64 GetHash() const { return GetDescription().GetHash(); }
      void Serialize(IO::Serializer* aSerializer);

      uint32 getNumSubModels() const {return m_vSubModels.size();}
    //---------------------------------------------------------------------------//
      void addSubModel(SharedPtr<SubModel>& _pSubModel);
    //---------------------------------------------------------------------------//
      SubModel* getSubModel(uint32 u32Index) {return m_vSubModels[u32Index].get();}
      const SubModel* getSubModel(uint32 u32Index) const {return m_vSubModels[u32Index].get();}
    //---------------------------------------------------------------------------//
      std::vector<SharedPtr<SubModel>>& getSubModelList() {return m_vSubModels;}
      const std::vector<SharedPtr<SubModel>>& getSubModelList() const {return m_vSubModels;}
    //---------------------------------------------------------------------------//
      void setSubModelList(const std::vector<SharedPtr<SubModel>>& _vSubModels)
      {ASSERT(m_vSubModels.empty()); m_vSubModels = _vSubModels; }
    //---------------------------------------------------------------------------//
    private:
      std::vector<SharedPtr<SubModel>> m_vSubModels;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Model)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

