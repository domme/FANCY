#ifndef INCLUDE_SCENENODECOMPONENT_H
#define INCLUDE_SCENENODECOMPONENT_H

#include "ObjectName.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class SceneNode;
  class SceneRenderDescription;
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNodeComponent
  {
    public:
      SceneNodeComponent(SceneNode* pOwner);
      virtual ~SceneNodeComponent();

      SceneNode* getSceneNode() { return m_pOwner; }
      
      virtual void update() = 0;
      virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) = 0;
      virtual ObjectName getTypeName() = 0;

    protected:
      SceneNode* m_pOwner;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNodeComponent)
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  template<class T, class ArgT>
  class BaseCreator
  {
  public: 
    static std::shared_ptr<T> create(ArgT arg) { return std::make_shared<T>(arg); }
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENENODECOMPONENT_H