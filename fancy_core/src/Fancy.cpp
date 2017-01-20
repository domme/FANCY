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
#include "RenderView.h"
#include "GraphicsWorld.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::ourInstance = nullptr;
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

    myMainWorld = std::make_shared<GraphicsWorld>();
    myMainView = new RenderView(anAppInstanceHandle, static_cast<uint32>(someParams.myRenderingTechnique), myMainWorld);
    myViews.push_back(myMainView.Get());
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
    for (RenderView* view : myViews)
      view->Startup();
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    ourInstance = new FancyRuntime(anAppInstanceHandle, someParams);
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();

    myRealTimeClock.Update(_dt);
    
    

    

    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy