#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialPass
  {
    SOLID_GBUFFER = 0,
    SOLID_FORWARD,
    TRANSPARENT_FORWARD,
    // More to come...
    NUM
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Material
  {
    class MaterialPass;

    public:
      Material();
      ~Material();

      const ObjectName& getName() const { return m_Name; }
      const MaterialPass* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint) ePassType]; }

    private:
      ObjectName m_Name;
      MaterialPass* m_vPasses[ (uint) EMaterialPass::NUM ];
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H