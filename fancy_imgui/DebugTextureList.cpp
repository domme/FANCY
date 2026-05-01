#include "DebugTextureList.h"
#include "EASTL/hash_map.h"
//
#include "Common/Ptr.h"
#include "IO/Assets.h"
#include "Rendering/Texture.h"
#include "Rendering/RenderCore.h"
#include "imgui.h"

namespace Fancy {
  void DebugTextureList::Update() {
    // ImGui::Begin("Textures", &myIsOpen, ImGuiWindowFlags_NoSavedSettings);

    const auto &                  textures = Assets::GetTextures();
    eastl::vector< const char * > textureNames;
    textureNames.reserve( textures.size() );

    ImGui::InputText( "Filter", myFilterText, ARRAY_LENGTH( myFilterText ) );
    const eastl::string filterName = myFilterText;

    int selectedIdx = -1;
    for ( const auto & it : textures ) {
      if ( !it.second.IsValid() )
        continue;
      const eastl::string & texName = RenderCore::GetTextureView( it.second )->GetTexture()->myName;
      if ( filterName.empty() || texName.find( filterName ) != eastl::string::npos ) {
        textureNames.push_back( texName.c_str() );
        if ( selectedIdx < 0 && textureNames.back() == mySelectedItem ) {
          selectedIdx = ( int ) textureNames.size() - 1;
        }
      }
    }

    if ( ImGui::ListBox( "Textures List", &selectedIdx, textureNames.data(), ( int ) textureNames.size() ) ) {
      mySelectedDebugImage.reset();

      mySelectedItem = textureNames[ selectedIdx ];
      TextureViewHandle selectedTexture = Assets::GetTexture( mySelectedItem.c_str() );

      if ( selectedTexture.IsValid() ) {
        mySelectedDebugImage.reset( new ImGuiMippedDebugImage(
            RenderCore::GetTextureView( selectedTexture )->GetTexture(), mySelectedItem.c_str() ) );
      }
    }

    if ( mySelectedDebugImage ) {
      mySelectedDebugImage->Update();
    }

    // ImGui::End();
  }
}  // namespace Fancy
