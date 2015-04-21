#ifndef INCLUDE_MODELCOMPONENT_H
#define INCLUDE_MODELCOMPONENT_H

#include "SceneNodeComponent.h"
#include "Model.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
	struct ModelComponentDesc : public SceneNodeComponentDesc
	{
		
	};
//---------------------------------------------------------------------------//
	DECLARE_SMART_PTRS(ModelComponentDesc);
//---------------------------------------------------------------------------//
  class DLLEXPORT ModelComponent : 
    public SceneNodeComponent, public BaseCreator<ModelComponent, SceneNode*>
  {
    public:
      ModelComponent(SceneNode* pOwner);
      virtual ~ModelComponent();

      void setModel(Geometry::Model* pModel) {m_pModel = pModel;}
      Geometry::Model* getModel() {return m_pModel;}
      
      virtual ObjectName getTypeName() const override { return _N(Model); }
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