#include "Material.h"
#include "Serializer.h"
#include "GraphicsWorld.h"
#include "Texture.h"
#include "RenderCore.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  Material::Material()
  {
    
  }
//---------------------------------------------------------------------------//
  Material::~Material()
  {

  }
//---------------------------------------------------------------------------//
  bool Material::operator==(const MaterialDesc& aDesc) const 
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  MaterialDesc Material::GetDescription() const
  {
    MaterialDesc desc;

    desc.myTextures.reserve(myTextures.size());
    for (const MaterialTexture& tex : myTextures)
    {
      MaterialTextureDesc texDesc;
      texDesc.mySemantic = static_cast<uint>(tex.mySemantic);
      texDesc.myTexture = tex.myTexture->GetDescription();
      desc.myTextures.push_back(texDesc);
    }

    desc.myParameters.reserve(myParameters.size());
    for (const MaterialParameter& param : myParameters)
    {
      MaterialParameterDesc paramDesc;
      paramDesc.mySemantic = static_cast<uint>(param.mySemantic);
      paramDesc.myValue = param.myValue;
      desc.myParameters.push_back(paramDesc);
    }

    return desc;
  }
//---------------------------------------------------------------------------//
  void Material::SetFromDescription(const MaterialDesc& aDesc, GraphicsWorld* aWorld)
  {
    if (aDesc == GetDescription())
      return;

    myTextures.resize(aDesc.myTextures.size());
    for (uint i = 0u; i < aDesc.myTextures.size(); ++i)
    {
      const MaterialTextureDesc& texDesc = aDesc.myTextures[i];
      myTextures[i].mySemantic = static_cast<EMaterialTextureSemantic>(texDesc.mySemantic);
      myTextures[i].myTexture = RenderCore::CreateTexture(texDesc.myTexture);
    }

    myParameters.resize(aDesc.myParameters.size());
    for (uint i = 0u; i < aDesc.myParameters.size(); ++i)
    {
      const MaterialParameterDesc& paramDesc = aDesc.myParameters[i];
      myParameters[i].mySemantic = static_cast<EMaterialParameterSemantic>(paramDesc.mySemantic);
      myParameters[i].myValue = paramDesc.myValue;
    }
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering