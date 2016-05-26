#include "Fancy.h"

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
#include "TextureLoader.h"
#include "ScopedPtr.h"
#include "RenderWindow.h"

namespace Fancy {
  Scene::ScenePtr m_pCurrScene = nullptr;
  ScopedPtr<Rendering::RenderingProcess> m_pRenderingProcess;
  Rendering::RenderOutput* ourRenderOutput = nullptr;
  HINSTANCE ourAppInstanceHandle = nullptr;

  void initComponentSubsystem()
  {
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::registerFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);
  }

  void initRenderingSubsystem()
  {
    Rendering::RenderCore::InitPlatform();
    Rendering::RenderCore::Init();

    ourRenderOutput = new Rendering::RenderOutput();
    ourRenderOutput->postInit();

    Rendering::RenderCore::PostInit();
  }

  void initIOsubsystem()
  {
    IO::PathService::SetResourceLocation("../../../resources/");
    IO::SceneImporter::initLogger();
  }

  bool Init(HINSTANCE anAppInstanceHandle)
  {
    ourAppInstanceHandle = anAppInstanceHandle;

    initIOsubsystem();
    initComponentSubsystem();
    initRenderingSubsystem();
    
    return true;
  }

  void ShutdownRenderingSubsystem()
  {
    Rendering::RenderCore::Shutdown();
    Rendering::RenderCore::ShutdownPlatform();

    SAFE_DELETE(ourRenderOutput);
  }
//---------------------------------------------------------------------------//
  void Shutdown()
  {
    IO::SceneImporter::destroyLogger();
    ShutdownRenderingSubsystem();
  }
//---------------------------------------------------------------------------//
  Rendering::RenderOutput* GetCurrentRenderOutput()
  {
    return ourRenderOutput;
  }
//---------------------------------------------------------------------------//
  Rendering::RenderingProcess* GetRenderingProcess()
  {
    return m_pRenderingProcess;
  }
//---------------------------------------------------------------------------//
  void Startup()
  {
    ASSERT(m_pCurrScene, "No scene set");
    ASSERT(m_pRenderingProcess, "No rendering process set");

    m_pRenderingProcess->Startup();
    m_pCurrScene->startup();
  }
//---------------------------------------------------------------------------//
  void Update(double _dt)
  {
    ASSERT(m_pCurrScene, "No scene set");
    ASSERT(m_pRenderingProcess, "No rendering process set");

    Time::update(_dt);
    const float deltaTime = Time::getDeltaTime();

    m_pCurrScene->update(deltaTime);

    ourRenderOutput->beginFrame();
    m_pRenderingProcess->Tick(deltaTime);
    ourRenderOutput->endFrame();
  }
//---------------------------------------------------------------------------//
  void SetCurrentScene( const Scene::ScenePtr& _pScene )
  {
    m_pCurrScene = _pScene;
  }
//---------------------------------------------------------------------------//
  const Scene::ScenePtr& GetCurrentScene()
  {
    return m_pCurrScene;
  }
//---------------------------------------------------------------------------//
  void SetRenderingProcess( Rendering::RenderingProcess* _pRenderingProcess )
  {
    if (m_pRenderingProcess == _pRenderingProcess)
      return;

    m_pRenderingProcess = _pRenderingProcess;
    m_pRenderingProcess->Startup();
  }
//---------------------------------------------------------------------------//
  HINSTANCE GetAppInstanceHandle()
  {
    return ourAppInstanceHandle;
  }

//---------------------------------------------------------------------------//
  Fancy::RenderWindow* GetCurrentRenderWindow()
  {
    if (ourRenderOutput != nullptr) 
      return ourRenderOutput->GetWindow(); 
    
    return nullptr;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy