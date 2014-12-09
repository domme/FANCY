#include "EngineCommon.h"

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  EngineCommon::~EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  bool EngineCommon::initEngine()
  {
    Scene::Startup::initComponentSubsystem();

    Rendering::Renderer::getInstance();


    return true;
  }
//---------------------------------------------------------------------------//
  EngineCommon::EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy