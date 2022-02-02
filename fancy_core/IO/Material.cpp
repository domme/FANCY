#include "fancy_core_precompile.h"
#include "Material.h"
#include "Rendering/Texture.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  uint64 MaterialDesc::GetHash() const
  {
    uint64 hash = 0u;
    for (const eastl::string& tex : myTextures)
      MathUtil::hash_combine(hash, tex);

    for (const glm::float4 param : myParameters)
      MathUtil::hash_combine(hash, MathUtil::ByteHash(param));

    return hash;
  }
//---------------------------------------------------------------------------//
  MaterialDesc Material::GetDescription() const
  {
    MaterialDesc desc;
    for (uint i = 0; i < ARRAY_LENGTH(myTextures); ++i)
      if (myTextures[i])
        desc.myTextures[i] = myTextures[i]->GetTexture()->GetProperties().myPath;

    for (uint i = 0; i < ARRAY_LENGTH(myParameters); ++i)
      desc.myParameters[i] = myParameters[i];

    return desc;
  }
//---------------------------------------------------------------------------//
}


