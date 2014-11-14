#ifndef INCLUDE_TRANSFORMCOMPONENT_H
#define INCLUDE_TRANSFORMCOMPONENT_H

#include "SceneNodeComponent.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class DLLEXPORT TransformComponent : 
    public SceneNodeComponent, public BaseCreator<TransformComponent, SceneNode*>
  {
    friend class SceneNode;

    public:
      TransformComponent(SceneNode* pOwner);
      virtual ~TransformComponent();

      virtual void update() override;
      virtual ObjectName getTypeName() override { return _N(Transform); }

      const glm::mat4& getLocalTransform() { return m_LocalTransform; }
      const glm::mat4& getCachedWorldTransform() { return m_CachedWorldTransform; }

    private:
      glm::mat4 m_LocalTransform;
      glm::mat4 m_CachedWorldTransform;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(TransformComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_TRANSFORMCOMPONENT_H