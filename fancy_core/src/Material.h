#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  class Material
  {
    public:
      Material();
      ~Material();

      const ObjectName& getName() const { return m_Name; }
      const MaterialPassInstance* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint) ePassType]; }

    private:
      ObjectName m_Name;
      MaterialPassInstance* m_vPasses[ (uint) EMaterialPass::NUM ];
  };
//---------------------------------------------------------------------------//
  // TODO: Do we need this?
  /*class MaterialInstance 
  {


  }; */
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H