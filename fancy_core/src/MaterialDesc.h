#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "DescriptionBase.h"
#include "TextureDesc.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialParameterSemantic
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class EMaterialTextureSemantic
  {
    BASE_COLOR = 0,
    NORMAL,
    MATERIAL,

    NUM,
    NONE = ~0
  };
//---------------------------------------------------------------------------//
  struct MaterialTextureDesc
  {
    MaterialTextureDesc() : mySemantic(~0u) {}
    MaterialTextureDesc(uint aSemantic, const TextureDesc& aTextureDesc) : mySemantic(aSemantic), myTexture(aTextureDesc) {}
    void Serialize(IO::Serializer* aSerializer);

    uint mySemantic;
    TextureDesc myTexture;
  };
//---------------------------------------------------------------------------//
  struct MaterialParameterDesc
  {
    MaterialParameterDesc() : mySemantic(~0u), myValue(0.0f) {}
    MaterialParameterDesc(uint aSemantic, float aValue) : mySemantic(aSemantic), myValue(aValue) {}
    void Serialize(IO::Serializer* aSerializer);

    uint mySemantic;
    float myValue;
  };
//---------------------------------------------------------------------------//
  struct MaterialDesc : public DescriptionBase
  {
    MaterialDesc();
    ~MaterialDesc() override {}

    bool operator==(const MaterialDesc& anOther) const;
    uint64 GetHash() const override;
    void Serialize(IO::Serializer* aSerializer) override;
    ObjectName GetTypeName() const override { return _N(Material); }
    bool IsEmpty() const override;

    std::vector<MaterialTextureDesc> myTextures;
    std::vector<MaterialParameterDesc> myParameters;
  };
//---------------------------------------------------------------------------//
} }

