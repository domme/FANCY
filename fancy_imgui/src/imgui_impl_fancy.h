#pragma once

class ImDrawData;

namespace Fancy {
    class FancyRuntime;
    class RenderWindow;
}

namespace Fancy { namespace ImGui {
  bool Init(Fancy::RenderWindow* aRenderWindow, Fancy::FancyRuntime* aRuntime);
  void NewFrame();
  void RenderDrawLists(ImDrawData* _draw_data);
} }

