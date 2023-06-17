#include "DebugTextureList.h"
#include "EASTL/hash_map.h"
// 
#include "Common/Ptr.h"
#include "IO/Assets.h"
#include "Rendering/Texture.h"
#include "imgui.h"

namespace Fancy
{
  class TextureView;

  void DebugTextureList::Update()
  {
    const eastl::hash_map<uint64, SharedPtr<TextureView>>& textures = Assets::GetTextures();
    eastl::vector<const char*> textureNames;
    textureNames.reserve(textures.size());
    
    int selectedIdx = -1;
    for (const auto& it : textures)
    {
      textureNames.push_back(it.second->myName.c_str());
      if (selectedIdx < 0 && textureNames.back() == mySelectedItem)
      {
        selectedIdx = textureNames.size() - 1;
      }
    }

    ImGui::Begin("Textures", &myIsOpen);

    if (ImGui::ListBox("Textures List", &selectedIdx, textureNames.data(), textureNames.size()))
    {
      mySelectedDebugImage.reset();

      mySelectedItem = textureNames[selectedIdx];
      SharedPtr<TextureView> selectedTexture = Assets::GetTexture(mySelectedItem.c_str());

      if ( selectedTexture )
      {
        mySelectedDebugImage.reset(new ImGuiMippedDebugImage(selectedTexture->GetTexturePtr(), mySelectedItem.c_str()));
      }
    }

    if ( mySelectedDebugImage )
    {
      mySelectedDebugImage->Update();
    }

    ImGui::End();
  }
}


