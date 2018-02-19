#include <windows.h>

#include <Fancy_Include.h>

#include "imgui.h"
#include "imgui_impl_fancy.h"

Fancy::Scene::CameraComponent* pCameraComponent;
Fancy::Scene::SceneNode* pModelNode;
Fancy::FancyRuntime* pRuntime = nullptr;

void OnWindowResized(Fancy::uint aWidth, Fancy::uint aHeight)
{
  pCameraComponent->setProjectionPersp(45.0f, aWidth, aHeight, 1.0f, 1000.0f);
}

void StartupEngine(HINSTANCE anAppInstanceHandle)
{
  if (pRuntime != nullptr)
    return;

  Fancy::RenderingStartupParameters params;
  params.myRenderingTechnique = Fancy::RenderingTechnique::FORWARD_PLUS;

  pRuntime = Fancy::FancyRuntime::Init(anAppInstanceHandle, params);

  Fancy::Window* window = pRuntime->GetMainWindow();
  std::function<void(Fancy::uint, Fancy::uint)> onResizeCallback = &OnWindowResized;
  window->myOnResize.Connect(onResizeCallback);
  
  Fancy::Scene::Scene* pScene = pRuntime->GetMainWorld()->GetScene();
  Fancy::Scene::SceneNode* pCameraNode = pScene->getRootNode()->createChildNode(_N(CameraNode));
  pCameraComponent = static_cast<Fancy::Scene::CameraComponent*>(pCameraNode->addOrRetrieveComponent(_N(CameraComponent)));
  pCameraComponent->setProjectionPersp(45.0f, window->GetWidth(), window->GetHeight(), 1.0f, 1000.0f);
  pScene->setActiveCamera(pCameraComponent);

  pModelNode = pRuntime->GetMainWorld()->Import("Models/cube.obj");
  pModelNode->getTransform().setPositionLocal(glm::vec3(0.0f, 0.0f, 10.0f));

  Fancy::Scene::SceneNode* pLightNode = pScene->getRootNode()->createChildNode(_N(LightNode));
  Fancy::Scene::LightComponent* pLight = static_cast<Fancy::Scene::LightComponent*>(pLightNode->addOrRetrieveComponent(_N(LightComponent)));
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  StartupEngine(hInstance);
  Fancy::ImGui::Init(pRuntime->GetMainWindow(), pRuntime);

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
  
    pRuntime->BeginFrame();
    Fancy::ImGui::NewFrame();

    pModelNode->getTransform().rotate(glm::vec3(1.0f, 1.0f, 0.0f), 20.0f);
    pRuntime->Update(0.016f);
    
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
    
    ImGui::Render();

    pRuntime->EndFrame();
  }
  
  Fancy::ImGui::Shutdown();
  Fancy::FancyRuntime::Shutdown();
  pRuntime = nullptr;

  return 0;
}