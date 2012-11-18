#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "../includes.h"
#include "Light.h"
//


class DLLEXPORT DirectionalLight : public Light
{
	friend class GLVolumeRenderer;

	public:
		DirectionalLight();
		virtual ~DirectionalLight();

		virtual void RenderShadowMap();
		virtual void Update();

		const glm::vec3& getDirection() const				{ return m_v3Direction; }
		void setDirection( const glm::vec3& v3Direction )	{ m_v3Direction = v3Direction; }

	protected:
		glm::vec3				m_v3Direction;
		

		
};

#endif