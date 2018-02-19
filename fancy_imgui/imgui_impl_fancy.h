#pragma once

struct ImDrawData;

namespace Fancy 
{
  class FancyRuntime;
  class Window;
}

namespace Fancy { namespace ImGui {
  bool Init(Fancy::Window* aWindow, Fancy::FancyRuntime* aRuntime);
  void NewFrame();
  void RenderDrawLists(ImDrawData* _draw_data);
  void Shutdown();
} }

