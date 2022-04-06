#pragma once
#include "Rendering/RenderOutput.h"
#include "Common/Ptr.h"

struct ImDrawData;

namespace Fancy
{
  struct VertexInputLayout;
  class TextureSampler;
  class DepthStencilState;
  class BlendState;
  class ShaderPipeline;
  class RenderOutput;
  class TextureView;

  namespace ImGuiRendering
  {
    bool Init(const SharedPtr<RenderOutput>& aRenderOutput);
    void NewFrame();
    void RenderDrawLists(ImDrawData* _draw_data);
    void Shutdown();
  };
} 

