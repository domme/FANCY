#pragma once

#include "Rendering/ResourceHandle.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/string.h"

namespace Fancy {
  class TextureView;
  class Texture;

  struct ImGuiDebugImage {
    float myZoom = 1.0f;
    void Update( Fancy::TextureView * aTexture, const char * aName );
  };

  struct ImGuiMippedDebugImage {
    ImGuiMippedDebugImage( Texture * aTexture, const char * aName );
    ~ImGuiMippedDebugImage();

    Texture * myTexture = nullptr;
    eastl::fixed_vector< TextureViewHandle, 16 > myMipViews;
    eastl::string myName;

    float myZoom = 1.0f;
    int myMipLevel = 0;
    void Update();
  };
}  // namespace Fancy