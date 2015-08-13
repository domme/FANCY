#include "Material.h"
#include "Serializer.h"
#include "MaterialPass.h"

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
  void Material::serialize(IO::Serializer* aSerializer)
  {
    aSerializer->serialize(&m_Name, "m_Name");
    aSerializer->serialize(&m_vParameters, "m_vParameters");
    aSerializer->serialize(&m_vPasses, "m_vPasses");
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering