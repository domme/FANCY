#pragma once
#include "Rendering/ResourceHandle.h"

struct ImDrawData;

namespace Fancy {
  struct VertexInputLayout;
  class TextureSampler;
  class DepthStencilState;
  class BlendState;
  class ShaderPipeline;
  class RenderOutput;
  class TextureView;

  namespace ImGuiRendering {
    bool Init( RenderOutputHandle aRenderOutput );
    void NewFrame();
    void RenderDrawLists( ImDrawData * _draw_data );
    void Shutdown();
  };  // namespace ImGuiRendering
}  // namespace Fancy
