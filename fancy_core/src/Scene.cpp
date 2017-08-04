#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "LightComponent.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  Scene::Scene()
    : m_pActiveCamera(nullptr)
  {
    m_pRootNode = FANCY_NEW(SceneNode(this), MemoryCategory::GENERAL);
  }
//---------------------------------------------------------------------------//
  Scene::~Scene()
  {
    FANCY_DELETE(m_pRootNode, MemoryCategory::GENERAL);
    m_pRootNode = nullptr;
  }
//---------------------------------------------------------------------------//
  void Scene::update(float _dt)
  {
    myLights.clear();
    myModels.clear();
    auto componentCallbackFn = [this](SceneNodeComponent* aComponent)
    {
      const ObjectName& typeName = aComponent->getTypeName();
      if (typeName == _N(LightComponent))
        myLights.push_back(static_cast<const LightComponent*>(aComponent));
      else if (typeName == _N(ModelComponent))
        myModels.push_back(static_cast<const ModelComponent*>(aComponent));
    };

    m_pRootNode->update(_dt, componentCallbackFn);
  }
//---------------------------------------------------------------------------//
  void Scene::startup()
  {
    m_pRootNode->startup();
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene