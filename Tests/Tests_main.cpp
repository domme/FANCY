#include <Windows.h>
#include "Common/FancyCoreDefines.h"
#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"
#include "Common/Fancy.h"
#include "Common/Window.h"
#include "Debug/Profiler.h"
#include <array>

#include "TestApplication.h"
#include "Common/Ptr.h"
#include "Common/StringUtil.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\"; }

using namespace Fancy;

Fancy::UniquePtr<TestApplication> myTestApp;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  int numArgs = 0;
  LPWSTR* commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &numArgs);

  eastl::vector<eastl::string> commandLineArgStrings(numArgs);
  eastl::vector<const char*> cStrings(numArgs);
  for (uint i = 0u; i < numArgs; ++i)
  {
    commandLineArgStrings[i] = StringUtil::ToNarrowString(commandLineArgs[i]);
    cStrings[i] = commandLineArgStrings[i].c_str();
  }

  LocalFree(commandLineArgs);

  RenderPlatformProperties renderProperties;
  WindowParameters windowParams;
  windowParams.myWidth = 1280;
  windowParams.myHeight = 720;
  myTestApp.reset(new TestApplication(hInstance, cStrings.data(), cStrings.size(), "Tests", renderProperties, windowParams));

  MSG msg = { 0 };
  while (true)
  {
    // Process any messages in the queue.
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      if (msg.message == WM_QUIT)
        break;
    }

    myTestApp->BeginFrame();
    myTestApp->Update();
    myTestApp->Render();
    myTestApp->EndFrame();
  }

  myTestApp.reset();

  return 0;
}