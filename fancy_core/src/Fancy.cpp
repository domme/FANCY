#include "Fancy.h"

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Renderer.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "PathService.h"
#include "SceneImporter.h"
#include "TimeManager.h"
#include "RenderingProcess.h"
#include "LightComponent.h"
#include "ScopedPtr.h"
#include "RenderingProcessForward.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams)
    : myAppInstanceHandle(anAppInstanceHandle)
    , myFrameIndex(0u)
  {
    // Init IO-subsystem
    IO::PathService::SetResourceLocation(someParams.myResourceFolder);
    IO::SceneImporter::initLogger();

    // Init Component subsystem
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::registerFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);

    // Init rendering subsystem
    if (!Rendering::RenderCore::IsInitialized())
    {
      Rendering::RenderCore::InitPlatform();
      Rendering::RenderCore::Init();
      Rendering::RenderCore::PostInit();
    }
    
    myRenderOutput = new Rendering::RenderOutput(anAppInstanceHandle);
    myRenderOutput->postInit();
    
    // Init scene
    myScene = new Scene::Scene();

    // Init Rendering process
    switch(someParams.myRenderingTechnique)
    {
      case RenderingTechnique::FORWARD: 
        myRenderingProcess = FANCY_NEW(Rendering::RenderingProcessForward, MemoryCategory::General);
        break;
      default: 
        ASSERT(false, "Unsupported rendering technique %", (uint32)someParams.myRenderingTechnique);
        break;
    }
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    IO::SceneImporter::destroyLogger();

    Rendering::RenderCore::Shutdown();
    Rendering::RenderCore::ShutdownPlatform();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::DoFirstFrameTasks()
  {
    myRenderingProcess->Startup();
    myScene->startup();
  }
//---------------------------------------------------------------------------//
  RenderWindow* FancyRuntime::GetCurrentRenderWindow()
  {
    return myRenderOutput->GetWindow();
  }
//---------------------------------------------------------------------------//
  Scene::SceneNode* FancyRuntime::Import(const std::string& aPath)
  {
    IO::SceneImporter importer(*myGraphicsWorld);
    Scene::SceneNode* node = myScene->getRootNode()->createChildNode();
    if (!importer.importToSceneGraph(aPath, node))
      return nullptr;

    return node;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();

    myRealTimeClock.Update(_dt);
    const float deltaTime = myRealTimeClock.GetDelta();

    myScene->update(deltaTime);

    myRenderOutput->beginFrame();
    myRenderingProcess->Tick(deltaTime);
    myRenderOutput->endFrame();

    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy