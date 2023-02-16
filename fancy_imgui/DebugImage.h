#pragma once

namespace Fancy
{
  class TextureView;

  struct ImGuiDebugImage
	{
		float myZoom = 1.0f;
		void Update(Fancy::TextureView* aTexture, const char* aName);
	};
}
