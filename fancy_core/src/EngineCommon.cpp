#include "EngineCommon.h"

#include "Scene.h"
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
#include "TimeManager.h"
#include "RenderingProcess.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Scene::ScenePtr EngineCommon::m_pCurrScene = nullptr;
  Rendering::RenderingProcess* EngineCommon::m_pRenderingProcess = nullptr;
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
  void EngineCommon::setWindowSize(uint32 _uWidth, uint32 _uHeight)
  {
    Rendering::Renderer& rend = Rendering::Renderer::getInstance();

    const glm::uvec4& viewportParams = rend.getViewport();
    if(viewportParams.z != _uWidth || viewportParams.w != _uHeight)
    {
      rend.setViewport(glm::uvec4(viewportParams.x, viewportParams.y, _uWidth, _uHeight));
    }
  }
//---------------------------------------------------------------------------//
  void EngineCommon::startup()
  {
    ASSERT_M(m_pCurrScene, "No scene set");
    ASSERT_M(m_pRenderingProcess, "No rendering process set");

    m_pRenderingProcess->startup();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::update(double _dt)
  {
    ASSERT_M(m_pCurrScene, "No scene set");
    ASSERT_M(m_pRenderingProcess, "No rendering process set");

    Time::update(_dt);
    const float deltaTime = Time::getDeltaTime();

    m_pCurrScene->update(deltaTime);

    Rendering::Renderer::getInstance().beginFrame();
    m_pRenderingProcess->tick(deltaTime);
    Rendering::Renderer::getInstance().endFrame();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::setCurrentScene( const Scene::ScenePtr& _pScene )
  {
    m_pCurrScene = _pScene;
  }
//---------------------------------------------------------------------------//
  const Scene::ScenePtr& EngineCommon::getCurrentScene()
  {
    return m_pCurrScene;
  }
//---------------------------------------------------------------------------//
  void EngineCommon::setRenderingProcess( Rendering::RenderingProcess* _pRenderingProcess )
  {
    m_pRenderingProcess = _pRenderingProcess;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy