#include "Material.h"
#include "Serializer.h"
#include "MaterialPass.h"
#include "MaterialPassInstance.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace Internal
  {
    String getParameterSemanticName(uint32 aSemantic)
    {
      static String names[] = {
        "DIFFUSE_REFLECTIVITY",
        "SPECULAR_REFLECTIVITY",
        "SPECULAR_POWER",
        "OPACITY"
      };

      static_assert(_countof(names) == (uint32)EMaterialParameterSemantic::NUM, "Missing names");

      return names[(uint32)aSemantic];
    }
  //---------------------------------------------------------------------------//
    String getMaterialPassName(uint32 aPass)
    {
      static String names[] = {
        "SOLID_GBUFFER",
        "SOLID_FORWARD",
        "TRANSPARENT_FORWARD"
      };

      static_assert(_countof(names) == (uint32)EMaterialPass::NUM, "Missing names");
      return names[(uint32)aPass];
    }
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  Material::Material()
  {
    // TODO: these can be changed to c-arrays again (serialization should support this by now...)

    m_vPasses.resize(m_vPasses.capacity());
    memset(&m_vPasses[0], 0u, sizeof(MaterialPassInstance*) * m_vPasses.capacity());

    m_vParameters.resize(m_vParameters.capacity());
    memset(&m_vParameters[0], 0x0, sizeof(float) * m_vParameters.capacity());
  }
//---------------------------------------------------------------------------//
  Material::~Material()
  {

  }
//---------------------------------------------------------------------------//
  bool Material::operator==(const Material& _other) const
  {
    bool same = memcmp(&m_vPasses[0], &_other.m_vPasses[0], 
      sizeof(MaterialPassInstance*) * m_vPasses.capacity()) == 0;

    if (same)
    {
      for (uint32 i = 0u; i < (uint32)EMaterialParameterSemantic::NUM; ++i)
      {
        same &= glm::abs(m_vParameters[i] - _other.m_vParameters[i]) < 1e-3;
      }
    }

    return same;
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

    for (uint i = 0u; i < m_vPasses.size(); ++i)
      if (m_vPasses[i] != nullptr)
        desc.myPasses[i] = m_vPasses[i]->GetDescription();

    for (uint i = 0u; i < m_vParameters.size(); ++i)
      desc.myParameters[i] = m_vParameters[i];

    return desc;
  }
//---------------------------------------------------------------------------//
  void Material::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_vParameters, "m_vParameters");
    aSerializer->serialize(&m_vPasses, "m_vPasses");
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering