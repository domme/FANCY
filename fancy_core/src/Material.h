#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"

#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  class Material : public StaticManagedHeapObject<Material>
  {
    public:
      Material();
      ~Material();

      const ObjectName& getName() const { return m_Name; }
      void setName(const ObjectName& _name) {m_Name = _name;}

      const MaterialPassInstance* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint32) ePassType]; }
      void setPass(const MaterialPassInstance* _pPass, EMaterialPass _ePassType) {m_vPasses[(uint32) _ePassType] = _pPass; }

    private:
      ObjectName m_Name;
      const MaterialPassInstance* m_vPasses[ (uint) EMaterialPass::NUM ];
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H