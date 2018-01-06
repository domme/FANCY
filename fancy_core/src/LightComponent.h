#ifndef INCLUDE_LIGHTCOMPONENT_H
#define INCLUDE_LIGHTCOMPONENT_H

#include "SceneNodeComponent.h"
#include "Serializable.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  enum class ELightType
  {
    DIRECTIONAL = 0,
    POINT,
    SPOT,
    AREA
  };
//---------------------------------------------------------------------------//
  class LightComponent final :
    public SceneNodeComponent, public BaseCreator<LightComponent>
  {
    public:
      SERIALIZABLE(LightComponent)

      LightComponent();
      ~LightComponent() override;

      ObjectName getTypeName() const override { return _N(LightComponent); }
      void Serialize(IO::Serializer* aSerializer) override;

      void init() override;
      void update() override;

      ELightType getType() const { return m_eType; }
      void setType(ELightType _type) { m_eType = _type; }

      const glm::vec3& getColorIntensity() const { return m_colorIntensity; }
      void setColorIntensity(const glm::vec3& _colorIntensity) { m_colorIntensity = _colorIntensity; }
      
      bool getCastsShadows() const { return m_castsShadows; }
      void setCastsShadows(bool _castsShadows) { m_castsShadows = _castsShadows; }

      float getFalloffStart() const { return m_falloffStart; }
      void setFalloffStart(float _falloffStart) { m_falloffStart = _falloffStart; }

      float getFalloffEnd() const { return m_falloffEnd; }
      void setFalloffEnd(float _falloffEnd) { m_falloffEnd = _falloffEnd; }

      float getConeAngle() const { return m_coneAngle; }
      void setConeAngle(float _coneAngle) { m_coneAngle = _coneAngle; }

    private:
      ELightType m_eType;
      glm::vec3 m_colorIntensity;
      bool m_castsShadows;
      float m_falloffStart;
      float m_falloffEnd;
      float m_coneAngle;
  };
  //---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(LightComponent)
  //---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_LIGHTCOMPONENT_H