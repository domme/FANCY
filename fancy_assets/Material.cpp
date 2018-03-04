#include "Material.h"
#include "fancy_core/Texture.h"

using namespace Fancy;

Material::Material()
{
  mySemanticTextures.fill(nullptr);
  mySemanticParameters.fill(FLT_MAX);
}

MaterialDesc Material::GetDescription() const
{
  MaterialDesc desc;
  for (int i = 0; i < mySemanticTextures.size(); ++i)
    if (mySemanticTextures[i])
      desc.mySemanticTextures[i] = mySemanticTextures[i]->GetDescription();

  for (const Texture* tex : myExtraTextures)
    desc.myExtraTextures.push_back(tex->GetDescription());

  for (int i = 0; i < mySemanticParameters.size(); ++i)
    desc.mySemanticParameters[i] = mySemanticParameters[i];

  for (float param : myExtraParameters)
    desc.myExtraParameters.push_back(param);

  return desc;
}
