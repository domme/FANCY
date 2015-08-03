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
      ModelComponent(SceneNode* pOwner);
      virtual ~ModelComponent();

      virtual ObjectName getTypeName() const override { return _N(ModelComponent); }
      virtual void serialize(IO::Serializer* aSerializer) override;

      void setModel(Geometry::Model* pModel) {m_pModel = pModel;}
      Geometry::Model* getModel() {return m_pModel;}
      
      virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) override;
      virtual void update() override;

    private:
      Geometry::Model* m_pModel;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(ModelComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_MODELCOMPONENT_H