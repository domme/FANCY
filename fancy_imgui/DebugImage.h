#pragma once

#include "Common/Ptr.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/string.h"

namespace Fancy
{
  class TextureView;
	class Texture;

  struct ImGuiDebugImage
	{
		float myZoom = 1.0f;
		void Update(Fancy::TextureView* aTexture, const char* aName);
	};

	struct ImGuiMippedDebugImage
	{
		ImGuiMippedDebugImage(SharedPtr<Texture>& aTexture, const char* aName);

		SharedPtr<Texture> myTexture;
		eastl::fixed_vector<SharedPtr<TextureView>, 16> myMipViews;
		eastl::string myName;

		float myZoom = 1.0f;
		int myMipLevel = 0;
		void Update();
	};
}
