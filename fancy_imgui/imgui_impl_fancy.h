#pragma once

struct ImDrawData;

namespace Fancy 
{
  class FancyRuntime;
}

namespace Fancy { namespace Rendering
{
  class RenderOutput;
} }

namespace Fancy { namespace ImGui {
  bool Init(Fancy::Rendering::RenderOutput* aRenderOutput, Fancy::FancyRuntime* aRuntime);
  void NewFrame();
  void RenderDrawLists(ImDrawData* _draw_data);
  void Shutdown();
} }

