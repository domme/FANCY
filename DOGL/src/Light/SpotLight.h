#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "../includes.h"
#include "Light.h"

class DLLEXPORT SpotLight : public Light
{
public:
	SpotLight();
	virtual ~SpotLight();

	float getUmbraAngle() const { return m_fUmbraAngle; }
	float getPenumbraAngle() const { return m_fPenumbraAngle; }
	float getExponent() const { return m_fExponent; }
	const glm::vec3& getCachedDirection() const { return m_v3CachedDirection; }

	virtual void update();
	virtual void onTransformChanged( const glm::mat4& newTransform );
	virtual void onAttatchedToNode();
	virtual void renderShadowMap();
	void setUmbraAngle( float fUmbra ) { m_fUmbraAngle = fUmbra; }
	void setUmbraAngleDeg( float fUmbraDeg ) { m_fUmbraAngle = glm::radians( fUmbraDeg ); }
	void setPenumbraAngle( float fPenumbra ) { m_fPenumbraAngle = fPenumbra; }
	void setPenumbraAngleDeg( float fPenumbraDeg ) { m_fPenumbraAngle = glm::radians( fPenumbraDeg ); }
	void setExponent( float fExp ) { m_fExponent = fExp; }
	void setCachedDirection( const glm::vec3& v3CachedDir ) { m_v3CachedDirection = v3CachedDir; }

protected:
	float		m_fUmbraAngle;
	float		m_fPenumbraAngle;
	float		m_fExponent;
	glm::vec3	m_v3CachedDirection;
};

#endif