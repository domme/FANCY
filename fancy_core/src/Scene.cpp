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
    m_pRootNode->update(_dt);
  }
//---------------------------------------------------------------------------//
  void Scene::startup()
  {
    m_pRootNode->startup();
  }
//---------------------------------------------------------------------------//
  void Scene::onComponentAdded(const SceneNodeComponent* _pComponent)
  {
    const ObjectName& typeName = _pComponent->getTypeName();
    if (typeName == _N(LightComponent))
    {
      myLights.push_back(static_cast<const LightComponent*>(_pComponent));
    }
    else if (typeName == _N(ModelComponent))
    {
      myModels.push_back(static_cast<const ModelComponent*>(_pComponent));
    }
  }
//---------------------------------------------------------------------------//
  void Scene::onComponentRemoved(const SceneNodeComponent* _pComponent)
  {
    const ObjectName& typeName = _pComponent->getTypeName();
    if (typeName == _N(LightComponent))
    {
      myLights.erase(static_cast<const LightComponent*>(_pComponent));
    }
    else if (typeName == _N(ModelComponent))
    {
      myModels.erase(static_cast<const ModelComponent*>(_pComponent));
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene