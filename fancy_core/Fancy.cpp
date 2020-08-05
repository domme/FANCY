#include "fancy_core_precompile.h"

#include "FancyCoreDefines.h"
#include "Fancy.h"

#include "RenderCore.h"

#include "PathService.h"
#include "TimeManager.h"
#include "Log.h"
#include "CommandQueue.h"
#include "RenderOutput.h"
#include "Profiler.h"
#include "CommandLine.h"
#include "ObjectCore.h"
#include "BinaryCache.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle)
    : myAppInstanceHandle(anAppInstanceHandle)
  {
    
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    RenderCore::Shutdown();
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const char** someArguments, uint aNumArguments, const WindowParameters& someWindowParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    CommandLine::CreateInstance(someArguments, aNumArguments);

    // Init IO-subsystem
    Path::InitRootFolders();

    ourInstance = new FancyRuntime(anAppInstanceHandle);

    // Init rendering subsystem
    if (!RenderCore::IsInitialized())
      RenderCore::Init();

    ASSERT(RenderCore::IsInitialized());

    ObjectCore::Init();

    // Create the output
    ourInstance->myRenderOutput = RenderCore::CreateRenderOutput(anAppInstanceHandle, someWindowParams);
    
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Shutdown()
  {
    RenderCore::GetCommandQueue(CommandListType::Graphics)->WaitForIdle();
    ourInstance->myRenderOutput.reset();

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
    Profiler::BeginFrame();
    RenderCore::BeginFrame();
    Profiler::BeginFrameGPU();
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
    Profiler::EndFrameGPU();
    RenderCore::EndFrame();
    Profiler::EndFrame();
    ++Time::ourFrameIdx;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy