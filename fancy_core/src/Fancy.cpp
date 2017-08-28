#include "Fancy.h"

#include "SceneNodeComponent.h"
#include "RenderCore.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "PathService.h"
#include "SceneImporter.h"
#include "TimeManager.h"
#include "LightComponent.h"
#include "ScopedPtr.h"
#include "RenderView.h"
#include "GraphicsWorld.h"
#include "EngineParameters.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  DLLEXPORT FancyRuntime* FancyRuntime::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle)
    : myAppInstanceHandle(anAppInstanceHandle)
    , myFrameIndex(0u)
  {
    
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    myMainView.reset();

    ASSERT(myMainWorld.unique(), "Dangling references to myMainWorld during shutdown");
    myMainWorld.reset();

    IO::SceneImporter::destroyLogger();
    Rendering::RenderCore::Shutdown();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Internal_Init(const EngineParameters& someParams)
  {
    myMainWorld = std::make_shared<GraphicsWorld>();
    myMainView = std::make_unique<RenderView>(myAppInstanceHandle, static_cast<uint32>(someParams.myRenderingTechnique), myMainWorld);
    myViews.push_back(myMainView.get());
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

    // Init IO-subsystem
    IO::ResourceUtil::InitResourceFolders();
    IO::SceneImporter::initLogger();

    // Init Component subsystem
    Scene::SceneNodeComponentFactory::RegisterFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::RegisterFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::RegisterFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::RegisterFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);

    ourInstance = new FancyRuntime(anAppInstanceHandle);

    // Init rendering subsystem
    if (!Rendering::RenderCore::IsInitialized())
      Rendering::RenderCore::Init(someParams.myRenderingApi);

    ASSERT(Rendering::RenderCore::IsInitialized());

    ourInstance->Internal_Init(someParams);

    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Shutdown()
  {
    ASSERT(ourInstance != nullptr);
    SAFE_DELETE(ourInstance);
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::GetInstance()
  {
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  RenderWindow* FancyRuntime::GetMainRenderWindow() const
  {
    return myMainView->GetRenderWindow();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::BeginFrame()
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();

    for (RenderView* view : myViews)
      view->BeginFrame();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    myRealTimeClock.Update(_dt);
   
    for (RenderView* view : myViews)
      view->Tick(myRealTimeClock);
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::EndFrame()
  {
    for (RenderView* view : myViews)
      view->EndFrame();

    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy