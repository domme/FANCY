#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"
#include "ObjectName.h"
#include "MaterialPass.h"
#include "Serializable.h"

namespace Fancy {
  namespace IO {
    class Serializer;
  }
}

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
  class Material : public StaticManagedHeapObject<Material>
  {
  public:
      SERIALIZABLE(Material)

      Material();
      ~Material();
      bool operator==(const Material& _other) const;

      static ObjectName getTypeName() { return _N(Material); }
      void serialize(IO::Serializer* aSerializer);

      const ObjectName& getName() const { return m_Name; }
      void setName(const ObjectName& _name) {m_Name = _name;}

      const MaterialPassInstance* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint32) ePassType]; }
      void setPass(MaterialPassInstance* _pPass, EMaterialPass _ePassType) {m_vPasses[(uint32) _ePassType] = _pPass; }

      float getParameter(EMaterialParameterSemantic _semantic) const { return m_vParameters[(uint32)_semantic]; }
      void setParameter(EMaterialParameterSemantic _semantic, float _value) { m_vParameters[(uint32)_semantic] = _value; }

    private:
      ObjectName m_Name;
      FixedArray<float, (uint32)EMaterialParameterSemantic::NUM> m_vParameters;
      FixedArray<MaterialPassInstance*, (uint32)EMaterialPass::NUM> m_vPasses;
      
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H