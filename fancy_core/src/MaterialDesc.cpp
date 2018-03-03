#include "MaterialDesc.h"
#include "MathUtil.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  void MaterialTextureDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&mySemantic, "mySemantic");
    aSerializer->Serialize(&myTexture, "myTexture");
  }
//---------------------------------------------------------------------------//
  void MaterialParameterDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&mySemantic, "mySemantic");
    aSerializer->Serialize(&myValue, "myValue");
  }
//---------------------------------------------------------------------------//
  MaterialDesc::MaterialDesc()
  {
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::operator==(const MaterialDesc& anOther) const 
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  uint64 MaterialDesc::GetHash() const
  {
    uint64 hash = 0u;
    
    for (const MaterialTextureDesc& textureDesc : myTextures)
    {
      MathUtil::hash_combine(hash, static_cast<uint>(textureDesc.mySemantic));
      MathUtil::hash_combine(hash, textureDesc.myTexture.GetHash());
    }

    for (const MaterialParameterDesc& paramDesc : myParameters)
    {
      MathUtil::hash_combine(hash, static_cast<uint>(paramDesc.mySemantic));
      MathUtil::hash_combine(hash, MathUtil::Hash(paramDesc.myValue));
    }

    return hash;
  }
//---------------------------------------------------------------------------//
  void MaterialDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myTextures, "myTextures");
    aSerializer->Serialize(&myParameters, "myParameters");
  }
//---------------------------------------------------------------------------//
  bool MaterialDesc::IsEmpty() const
  {
    return myTextures.empty() && myParameters.empty();
  }
//---------------------------------------------------------------------------//
} }
