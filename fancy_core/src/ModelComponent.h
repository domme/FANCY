#ifndef INCLUDE_MODELCOMPONENT_H
#define INCLUDE_MODELCOMPONENT_H

#include "SceneNodeComponent.h"
#include "Serializable.h"

namespace Fancy { namespace Geometry {
  class Model;
} }

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class DLLEXPORT ModelComponent : 
    public SceneNodeComponent, public BaseCreator<ModelComponent, SceneNode*>
  {
    public:
      explicit ModelComponent(SceneNode* pOwner);
      virtual ~ModelComponent();

      virtual ObjectName getTypeName() const override { return _N(ModelComponent); }
      virtual void Serialize(IO::Serializer* aSerializer) override;

      void setModel(const SharedPtr<Geometry::Model>& pModel) {m_pModel = pModel;}
      Geometry::Model* getModel() const { return m_pModel.get(); }
      
      void update() override;

    private:
      SharedPtr<Geometry::Model> m_pModel;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(ModelComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_MODELCOMPONENT_H