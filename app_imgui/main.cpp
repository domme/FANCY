#include <windows.h>

#include "App_Imgui.h"

std::unique_ptr<App_Imgui> myApp;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  myApp.reset(new App_Imgui(hInstance));

  myApp->Init();

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
  
    myApp->Update();
    myApp->Render();
  }

  myApp->Shutdown();
 
  return 0;
}