#include "EngineCommon.h"

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "Texture.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "PathService.h"
#include "SceneImporter.h"

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
    initRenderingSubsystem();
    initIOsubsystem();

    return true;
  }
//---------------------------------------------------------------------------//
  void EngineCommon::shutdownEngine()
  {
    IO::SceneImporter::destroyLogger();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initRenderingSubsystem()
  {
    // Init common blend- and depthstencil states
    Rendering::BlendState::init();
    Rendering::DepthStencilState::init();

    Rendering::Renderer& rend = Rendering::Renderer::getInstance();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initIOsubsystem()
  {
    IO::PathService::SetResourceLocation("../../../resources/");
    IO::SceneImporter::initLogger();
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy