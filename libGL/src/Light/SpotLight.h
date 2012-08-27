#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "../includes.h"
#include "Light.h"

class DLLEXPORT SpotLight : public Light
{
public:
	SpotLight();
	virtual ~SpotLight();

	float getUmbraAngle() const									{ return m_fUmbraAngle; }
	float getPenumbraAngle() const								{ return m_fPenumbraAngle; }
	float getExponent() const									{ return m_fExponent; }
	const glm::vec3& getDirection() const						{ return m_v3Direction; }

	virtual void update();
	virtual void renderShadowMap();
	void setUmbraAngle( float fUmbra )							{ m_fUmbraAngle = fUmbra; }
	void setUmbraAngleDeg( float fUmbraDeg )					{ m_fUmbraAngle = glm::radians( fUmbraDeg ); }
	void setPenumbraAngle( float fPenumbra )					{ m_fPenumbraAngle = fPenumbra; }
	void setPenumbraAngleDeg( float fPenumbraDeg )				{ m_fPenumbraAngle = glm::radians( fPenumbraDeg ); }
	void setExponent( float fExp )								{ m_fExponent = fExp; }
	void setDirection( const glm::vec3& v3Dir )					{ m_v3Direction = v3Dir; }

protected:
	float		m_fUmbraAngle;
	float		m_fPenumbraAngle;
	float		m_fExponent;
	glm::vec3	m_v3Direction;
};

#endif