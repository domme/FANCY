#include <windows.h>

#include "fancy_core/Fancy_Include.h"
#include <fancy_core/RenderCore.h>
#include "fancy_imgui/imgui.h"
#include "fancy_imgui/imgui_impl_fancy.h"
#include <fancy_core/RenderOutput.h>

Fancy::FancyRuntime* myRuntime = nullptr;
UniquePtr<Fancy::Rendering::RenderOutput> myRenderOutput;
Fancy::Window* myWindow = nullptr;

void OnWindowResized(Fancy::uint aWidth, Fancy::uint aHeight)
{
  // pCameraComponent->setProjectionPersp(45.0f, (float) aWidth, (float) aHeight, 1.0f, 1000.0f);
}

void StartupEngine(HINSTANCE anAppInstanceHandle)
{
  if (myRuntime != nullptr)
    return;

  Fancy::RenderingStartupParameters params;
  params.myRenderingTechnique = Fancy::RenderingTechnique::FORWARD;

  myRuntime = Fancy::FancyRuntime::Init(anAppInstanceHandle, params);
  myRenderOutput = Fancy::Rendering::RenderCore::CreateRenderOutput(anAppInstanceHandle);
  myWindow = myRenderOutput->GetWindow();
  
  std::function<void(Fancy::uint, Fancy::uint)> onResizeCallback = &OnWindowResized;
  myWindow->myOnResize.Connect(onResizeCallback);
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  StartupEngine(hInstance);
  Fancy::ImGui::Init(myRenderOutput.get(), myRuntime);

  bool show_test_window = true;
  bool show_another_window = false;
  ImVec4 clear_col = ImColor(114, 144, 154);

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
  
    myRuntime->BeginFrame();
    Fancy::ImGui::NewFrame();

    
    myRuntime->Update(0.016f);
    
    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
      static float f = 0.0f;
      ImGui::Text("Hello, world!");
      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
      ImGui::ColorEdit3("clear color", (float*)&clear_col);
      if (ImGui::Button("Test Window")) show_test_window ^= 1;
      if (ImGui::Button("Another Window")) show_another_window ^= 1;
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
      ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
      ImGui::Begin("Another Window", &show_another_window);
      ImGui::Text("Hello");
      ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
      ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
      ImGui::ShowTestWindow(&show_test_window);
    }
    
    // ImGui::Render();
    myRuntime->EndFrame();
  }
  
  Fancy::ImGui::Shutdown();
  Fancy::FancyRuntime::Shutdown();
  myRenderOutput = nullptr;
  myWindow = nullptr;
  myRuntime = nullptr;

  return 0;
}