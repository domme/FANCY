#pragma once
#include "fancy_core/FancyCorePrerequisites.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class Material;
  class Model;
  struct ModelDesc;
  struct MaterialDesc;
  class Texture;
  struct TextureDesc;
//---------------------------------------------------------------------------//
  namespace AssetManager
  {
    Material* CreateMaterial(const MaterialDesc& aDesc);
    Texture* CreateTexture(const TextureDesc& aDesc);
    Texture* CreateTexture(const char* aPath);
    Model* CreateModel(const ModelDesc& aDesc);
  }
//---------------------------------------------------------------------------//
}


