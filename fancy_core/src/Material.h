#ifndef INCLUDE_MATERIAL_H
#define INCLUDE_MATERIAL_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"
#include "ObjectName.h"
#include "MaterialPass.h"
#include "Serializable.h"
#include "MaterialDesc.h"

namespace Fancy {
  class GraphicsWorld;
}

namespace Fancy {
  namespace IO {
    class Serializer;
  }
}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class Material
  {
  public:
      SERIALIZABLE_RESOURCE(Material)

      Material();
      ~Material();
      bool operator==(const Material& _other) const;
      bool operator==(const MaterialDesc& aDesc) const;
      
      MaterialDesc GetDescription() const;
      void SetFromDescription(const MaterialDesc& aDesc, GraphicsWorld* aWorld);

      uint64 GetHash() const { return GetDescription().GetHash(); }
      
      const MaterialPassInstance* getPass(EMaterialPass ePassType) const { return m_vPasses[(uint32) ePassType]; }
      void setPass(MaterialPassInstance* _pPass, EMaterialPass _ePassType) {m_vPasses[(uint32) _ePassType] = _pPass; }

      float getParameter(EMaterialParameterSemantic _semantic) const { return m_vParameters[(uint32)_semantic]; }
      void setParameter(EMaterialParameterSemantic _semantic, float _value) { m_vParameters[(uint32)_semantic] = _value; }

    private:
      float m_vParameters [(uint32)EMaterialParameterSemantic::NUM];
      MaterialPassInstance* m_vPasses[(uint32)EMaterialPass::NUM];
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIAL_H