#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"
#include "ObjectName.h"
#include "MaterialPass.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialParameterSemantic
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM
  };
//---------------------------------------------------------------------------//  
  struct MaterialDesc
  {
    ObjectName myName;
    MaterialPassInstanceDesc myPasses[(uint32)EMaterialPass::NUM];
    float myParameters[(uint32)EMaterialParameterSemantic::NUM];
  };
//---------------------------------------------------------------------------//  
  class Material : public StaticManagedHeapObject<Material>
  {
    public:
      Material();
      ~Material();
      bool operator==(const Material& _other) const;

      MaterialDesc getDescription() const;
      void initFromDescription(const MaterialDesc& _aDesc);

      const ObjectName& getName() const { return m_Name; }
      void setName(const ObjectName& _name) {m_Name = _name;}

      const MaterialPassInstance* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint32) ePassType]; }
      void setPass(const MaterialPassInstance* _pPass, EMaterialPass _ePassType) {m_vPasses[(uint32) _ePassType] = _pPass; }

      float getParameter(EMaterialParameterSemantic _semantic) const { return m_vParameters[(uint32)_semantic]; }
      void setParameter(EMaterialParameterSemantic _semantic, float _value) { m_vParameters[(uint32)_semantic] = _value; }

    private:
      ObjectName m_Name;
      const MaterialPassInstance* m_vPasses[ (uint32) EMaterialPass::NUM ];
      float m_vParameters[(uint32)EMaterialParameterSemantic::NUM];
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H