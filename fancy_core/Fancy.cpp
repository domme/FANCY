#include "Fancy.h"

#include "RenderCore.h"

#include "PathService.h"
#include "TimeManager.h"
#include "RenderingStartupParameters.h"
#include "RenderOutput.h"

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
    RenderCore::Shutdown();
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const RenderingStartupParameters& someParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    // Init IO-subsystem
    Resources::InitResourceFolders();

    ourInstance = new FancyRuntime(anAppInstanceHandle);

    // Init rendering subsystem
    if (!RenderCore::IsInitialized())
      RenderCore::Init(someParams.myRenderingApi);

    ASSERT(RenderCore::IsInitialized());

    // Create the output
    ourInstance->myRenderOutput = RenderCore::CreateRenderOutput(anAppInstanceHandle);
    
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
  void FancyRuntime::DoFirstFrameTasks()
  {
    myRenderOutput->PrepareForFirstFrame();
  }
  //---------------------------------------------------------------------------//
  void FancyRuntime::BeginFrame()
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();

    myRenderOutput->BeginFrame();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    myRealTimeClock.Update(static_cast<float>(_dt));
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::EndFrame()
  {
    myRenderOutput->EndFrame();

    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy