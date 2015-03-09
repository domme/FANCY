#include "Scene.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  Scene::Scene()
  {
    m_pRootNode = FANCY_NEW(SceneNode, MemoryCategory::GENERAL);
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
  void Scene::gatherRenderItems( SceneRenderDescription* pRenderDesc )
  {
    m_pRootNode->gatherRenderItems(pRenderDesc);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene