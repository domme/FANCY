
#include "DebugImage.h"
#include "imgui.h"
#include "Common/MathIncludes.h"
#include "Rendering/Texture.h"

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
}

