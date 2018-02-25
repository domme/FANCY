#pragma once

struct ImDrawData;

namespace Fancy 
{
  class FancyRuntime;
  class RenderOutput;
}

namespace Fancy { namespace ImGuiRendering {
  bool Init(Fancy::RenderOutput* aRenderOutput, Fancy::FancyRuntime* aRuntime);
  void NewFrame();
  void RenderDrawLists(ImDrawData* _draw_data);
  void Shutdown();
} }

