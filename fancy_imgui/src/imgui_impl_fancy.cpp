#include "imgui_impl_fancy.h"

#include "imgui.h"

#include <Fancy_Include.h>

#include "GpuProgramPipeline.h"

Fancy::FancyRuntime* theFancyRuntime = nullptr;
Fancy::RenderWindow* theRenderWindow = nullptr;

HWND theHWND = nullptr;
INT64 theTicksPerSecond = 0;
INT64 theTime = 0;

std::shared_ptr<Fancy::Rendering::GpuProgramPipeline> theGpuProgramPipeline;

bool ImGui_ImplFancy_Init(Fancy::RenderWindow* aRenderWindow, Fancy::FancyRuntime* aRuntime)
{
  theRenderWindow = aRenderWindow;
  theFancyRuntime = aRuntime;
  theHWND = aRenderWindow->GetWindowHandle();

  ImGuiIO& io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = VK_TAB;                              // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
  io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
  io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
  io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
  io.KeyMap[ImGuiKey_Home] = VK_HOME;
  io.KeyMap[ImGuiKey_End] = VK_END;
  io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
  io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = 'A';
  io.KeyMap[ImGuiKey_C] = 'C';
  io.KeyMap[ImGuiKey_V] = 'V';
  io.KeyMap[ImGuiKey_X] = 'X';
  io.KeyMap[ImGuiKey_Y] = 'Y';
  io.KeyMap[ImGuiKey_Z] = 'Z';

  io.RenderDrawListsFn = ImGui_ImplFancy_RenderDrawLists;
  io.ImeWindowHandle = theHWND;

  if (!QueryPerformanceFrequency((LARGE_INTEGER *)&theTicksPerSecond))
    return false;
  if (!QueryPerformanceCounter((LARGE_INTEGER *)&theTime))
    return false;
  
  return true;
}

void ImGui_ImplFancy_NewFrame()
{
  ImGuiIO& io = ImGui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  RECT rect;
  GetClientRect(theHWND, &rect);
  io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

  // Setup time step
  INT64 current_time;
  QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
  io.DeltaTime = (float)(current_time - theTime) / theTicksPerSecond;
  theTime = current_time;

  // Read keyboard modifiers inputs
  io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
  io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
  io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
  // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
  // io.MousePos : filled by WM_MOUSEMOVE events
  // io.MouseDown : filled by WM_*BUTTON* events
  // io.MouseWheel : filled by WM_MOUSEWHEEL events

  // Hide OS mouse cursor if ImGui is drawing it
  SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

  // Start the frame
  ImGui::NewFrame();
}

void ImGui_ImplFancy_RenderDrawLists(ImDrawData* _draw_data)
{

}
