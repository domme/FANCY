
#include "DebugImage.h"
#include "imgui.h"
#include "Common/MathIncludes.h"
#include "Common/StaticString.h"
#include "Rendering/CommandList.h"
#include "Rendering/RenderCore.h"
#include "Rendering/Texture.h"
#include "Rendering/TextureData.h"

namespace Fancy
{
	void ImGuiDebugImage::Update(Fancy::TextureView* aTexture, const char* aName)
	{
		glm::float2 size = glm::float2((float)aTexture->GetTexture()->GetProperties().myWidth, (float)aTexture->GetTexture()->GetProperties().myHeight);
		size *= myZoom;

		ImGui::Text(aName);
		ImGui::Image((ImTextureID)aTexture, { size.x, size.y });
		ImGui::DragFloat("Zoom", &myZoom, 0.1f, 0.25f, 10.0f);
	}

  ImGuiMippedDebugImage::ImGuiMippedDebugImage(SharedPtr<Texture>& aTexture, const char* aName)
    : myTexture( aTexture )
	  , myName(aName)
  {
		const TextureProperties& props = aTexture->GetProperties();
		for ( int mip = 0; mip < props.myNumMipLevels; ++mip )
		{
			TextureViewProperties viewProps;
			viewProps.myFormat = props.myFormat;
			viewProps.mySubresourceRange = SubresourceRange(aTexture->GetSubresourceLocation(mip));
			myMipViews.push_back(RenderCore::CreateTextureView(aTexture, viewProps, StaticString<128>("%s_mip_%d", aName, mip)));
		}
  }

  void ImGuiMippedDebugImage::Update()
  {
		glm::float2 textureSize = glm::float2((float)myTexture->GetProperties().myWidth, (float)myTexture->GetProperties().myHeight);
		float ratio = textureSize.y / textureSize.x;

		glm::float2 availableSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		glm::float2 size = availableSize;
		if ( availableSize.y < availableSize.x )
		{
			size.x = size.y / ratio;
		} else
		{
			size.y = size.x * ratio;
		}

		size *= myZoom;

		myMipLevel = glm::min(myMipLevel + 1, (int) myMipViews.size()) - 1;
		if ( myMipLevel >= 0 )
		{
			TextureView* view = myMipViews[myMipLevel].get();
			ImGui::Text(myName.c_str());
			ImGui::Image((ImTextureID)view, { size.x, size.y });
			ImGui::DragFloat("Zoom", &myZoom, 0.1f, 0.25f, 2.0f);
			ImGui::DragInt("Mip", &myMipLevel, 1, 0, (int)myMipViews.size() - 1);
		}
  }
}

