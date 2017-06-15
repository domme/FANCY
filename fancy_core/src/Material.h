#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ObjectName.h"
#include "Serializable.h"
#include "MaterialDesc.h"

namespace Fancy {
  class GraphicsWorld;
}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct MaterialTexture
  {
    EMaterialTextureSemantic mySemantic;
    SharedPtr<Texture> myTexture;
  };
//---------------------------------------------------------------------------//
  struct MaterialParameter
  {
    EMaterialParameterSemantic mySemantic;
    float myValue;
  };
//---------------------------------------------------------------------------//
  class Material
  {
  public:
      SERIALIZABLE_RESOURCE(Material)

      Material();
      ~Material();
      bool operator==(const MaterialDesc& aDesc) const;
      
      MaterialDesc GetDescription() const;
      void SetFromDescription(const MaterialDesc& aDesc, GraphicsWorld* aWorld);

      uint64 GetHash() const { return GetDescription().GetHash(); }

      std::vector<MaterialParameter> myParameters;
      std::vector<MaterialTexture> myTextures;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering