#include "Fancy.h"

#include "RenderCore.h"

#include "PathService.h"
#include "TimeManager.h"
#include "RenderingStartupParameters.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle)
    : myAppInstanceHandle(anAppInstanceHandle)
    , myFrameIndex(0u)
  {
    
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    Rendering::RenderCore::Shutdown();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Internal_Init(const RenderingStartupParameters& someParams)
  {
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::DoFirstFrameTasks()
  {
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const RenderingStartupParameters& someParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    // Init IO-subsystem
    IO::Resources::InitResourceFolders();

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
  void FancyRuntime::BeginFrame()
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    myRealTimeClock.Update(static_cast<float>(_dt));
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::EndFrame()
  {
    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy