#include "EngineCommon.h"

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  EngineCommon::EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  EngineCommon::~EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  bool EngineCommon::initEngine()
  {
    initComponentSubsystem();

    Rendering::Renderer& rnd = Rendering::Renderer::getInstance();

    return true;
  }

//---------------------------------------------------------------------------//
  void EngineCommon::initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(Model), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(Camera), Scene::CameraComponent::create);
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy