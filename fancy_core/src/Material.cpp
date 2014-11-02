#include "Material.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  Material::Material()
  {
    memset(m_vPasses, 0u, sizeof(m_vPasses));
  }
//---------------------------------------------------------------------------//
  Material::~Material()
  {

  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering