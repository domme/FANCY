#include "App_Imgui.h"

#include <fancy_core/RendererPrerequisites.h>
#include <fancy_core/CommandListType.h>
#include "fancy_core/CommandContext.h"
#include "fancy_assets/ModelLoader.h"
#include "fancy_assets/GraphicsWorld.h"

using namespace Fancy;

bool show_test_window = true;
bool show_another_window = false;
ImVec4 clear_col = ImColor(114, 144, 154);

App_Imgui::App_Imgui(HINSTANCE anInstanceHandle)
{
  RenderingStartupParameters params;
  params.myRenderingTechnique = RenderingTechnique::FORWARD;

  myRuntime = FancyRuntime::Init(anInstanceHandle, params);
  myRuntime->GetRenderOutput()->GetWindow()->myOnResize.Connect(this, &App_Imgui::OnWindowResized);
}

App_Imgui::~App_Imgui()
{
}

void App_Imgui::Init()
{
  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);

  myGraphicsWorld.reset(new GraphicsWorld());

  ModelLoader::LoadResult loadResult;
  ModelLoader::LoadFromFile("Models/cube.obj", *myGraphicsWorld.get(), loadResult);
}

void App_Imgui::Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
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
}

void App_Imgui::Render()
{
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);
  
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRuntime->GetRenderOutput()->GetBackbuffer(), clearColor);
  
  ctx->ExecuteAndReset();
  RenderCore::FreeContext(ctx);

  ImGui::Render();
  myRuntime->EndFrame();
}

void App_Imgui::Shutdown()
{
  ImGuiRendering::Shutdown();
  FancyRuntime::Shutdown();
  myRuntime = nullptr;
}

void App_Imgui::OnWindowResized(Fancy::uint aWidth, Fancy::uint aHeight)
{
  
}
