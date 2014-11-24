#include "Scene.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  Scene::Scene()
  {
    
  }
//---------------------------------------------------------------------------//
  Scene::~Scene()
  {

  }
//---------------------------------------------------------------------------//
  void Scene::update()
  {

  }
//---------------------------------------------------------------------------//
  void Scene::gatherRenderItems( SceneRenderDescription* pRenderDesc )
  {
    m_pRootNode->gatherRenderItems(pRenderDesc);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene