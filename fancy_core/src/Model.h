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
  class Model
  {
    public:
      SERIALIZABLE_RESOURCE(Model)

      Model();
      ~Model();
      bool operator==(const Model& anOther) const;
      bool operator==(const ModelDesc& aDesc) const;

      ModelDesc GetDescription() const;
      void SetFromDescription(const ModelDesc& aDesc, GraphicsWorld* aWorld);

      uint64 GetHash() const { return GetDescription().GetHash(); }

      uint32 getNumSubModels() const {return m_vSubModels.size();}
    //---------------------------------------------------------------------------//
      SubModel* getSubModel(uint32 u32Index) {return m_vSubModels[u32Index].get();}
      const SubModel* getSubModel(uint32 u32Index) const {return m_vSubModels[u32Index].get();}
    //---------------------------------------------------------------------------//
      std::vector<SharedPtr<SubModel>>& getSubModelList() {return m_vSubModels;}
      const std::vector<SharedPtr<SubModel>>& getSubModelList() const {return m_vSubModels;}
    //---------------------------------------------------------------------------//
    private:
      std::vector<SharedPtr<SubModel>> m_vSubModels;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Model)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry

