#include "Test_ImGui.h"
#include "imgui.h"

Test_ImGui::Test_ImGui(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
  : Test(aRuntime, aWindow, aRenderOutput, anInputState, "ImGui")
{

}

void Test_ImGui::OnUpdate(bool aDrawProperties)
{
  if (aDrawProperties && ImGui::Button("Toggle ImGui Demo Window"))
    myShowTestWindow ^= 1;

  if (myShowTestWindow)
  {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
    ImGui::ShowDemoWindow(&myShowTestWindow);
  }
}
