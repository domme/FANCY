#include "EngineCommon.h"

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "Texture.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "PathService.h"
#include "SceneImporter.h"
#include "TimeManager.h"
#include "RenderingProcess.h"
#include "LightComponent.h"

namespace Fancy {
  Scene::ScenePtr m_pCurrScene = nullptr;
  Rendering::RenderingProcess* m_pRenderingProcess = nullptr;
  Rendering::Renderer* ourRenderer = nullptr;

//---------------------------------------------------------------------------//
  EngineCommon::EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  EngineCommon::~EngineCommon()
  {

  }
//---------------------------------------------------------------------------//
  bool EngineCommon::initEngine(void* aNativeWindowHandle)
  {
    initIOsubsystem();
    initComponentSubsystem();
    initRenderingSubsystem(aNativeWindowHandle);
    
    return true;
  }
//---------------------------------------------------------------------------//
  void EngineCommon::shutdownEngine()
  {
    IO::SceneImporter::destroyLogger();
    ShutdownRenderingSubsystem();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::registerFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);
  }
//---------------------------------------------------------------------------//
  void EngineCommon::initRenderingSubsystem(void* aNativeWindowHandle)
  {
    ourRenderer = new Rendering::Renderer(aNativeWindowHandle);
    
    Rendering::RenderingSubsystem::InitPlatform();
    Rendering::RenderingSubsystem::Init();
    
    ourRenderer->postInit();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::ShutdownRenderingSubsystem()
  {
    Rendering::RenderingSubsystem::Shutdown();
    Rendering::RenderingSubsystem::ShutdownPlatform();

    SAFE_DELETE(ourRenderer);
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
    const glm::uvec4& viewportParams = ourRenderer->GetDefaultContext()->getViewport();
    if(viewportParams.z != _uWidth || viewportParams.w != _uHeight)
    {
      ourRenderer->GetDefaultContext()->setViewport(glm::uvec4(viewportParams.x, viewportParams.y, _uWidth, _uHeight));
    }
  }
//---------------------------------------------------------------------------//
  Rendering::Renderer* EngineCommon::GetRenderer()
  {
    return ourRenderer;
  }
//---------------------------------------------------------------------------//
  Rendering::RenderingProcess* EngineCommon::GetRenderingProcess()
  {
    return m_pRenderingProcess;
  }
//---------------------------------------------------------------------------//
  void EngineCommon::startup()
  {
    ASSERT_M(m_pCurrScene, "No scene set");
    ASSERT_M(m_pRenderingProcess, "No rendering process set");

    m_pRenderingProcess->startup();
    m_pCurrScene->startup();
  }
//---------------------------------------------------------------------------//
  void EngineCommon::update(double _dt)
  {
    ASSERT_M(m_pCurrScene, "No scene set");
    ASSERT_M(m_pRenderingProcess, "No rendering process set");

    Time::update(_dt);
    const float deltaTime = Time::getDeltaTime();

    m_pCurrScene->update(deltaTime);

    ourRenderer->beginFrame();
    // m_pRenderingProcess->tick(deltaTime);
    ourRenderer->endFrame();
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