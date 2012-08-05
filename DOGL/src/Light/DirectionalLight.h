#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "../includes.h"
#include "Light.h"
//#include "Engine.h"


class DLLEXPORT DirectionalLight : public Light
{
	friend class GLVolumeRenderer;

	public:
		DirectionalLight();
		virtual ~DirectionalLight();

		virtual void renderShadowMap();
		virtual void update();

		const glm::vec3& getCachedDirection() const { return m_v3CachedDirection; }
		void setCachedDirection( const glm::vec3& v3Direction ) { m_v3CachedDirection = v3Direction; }

	protected:
		glm::vec3				m_v3CachedDirection;
		

		
};

#endif