#include "Material.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  Material::Material()
  {
    memset(m_vPasses, 0u, sizeof(m_vPasses));
    memset(m_vParameters, 0x0, sizeof(m_vParameters));
  }
//---------------------------------------------------------------------------//
  Material::~Material()
  {

  }
//---------------------------------------------------------------------------//
  bool Material::operator==(const Material& _other) const
  {
    bool same = memcmp(m_vPasses, _other.m_vPasses, sizeof(m_vPasses)) == 0;

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
} } // end of namespace Fancy::Rendering