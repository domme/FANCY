#ifndef LIGHT_H
#define LIGHT_H

#include "../includes.h"
#include "../Geometry/Mesh.h"
#include "../Scene/Camera.h"
//#include "OpenGL_Includes.h"
#include "../Rendering/Shader.h"
#include "../Rendering/Materials/Material.h"

class DLLEXPORT Light
{
public:

		enum ELightTpye
		{
			LIGHTTYPE_POINT,
			LIGHTTYPE_DIRECTIONAL,
			LIGHTTYPE_SPOT,
		};

		Light();
		virtual ~Light();

		virtual void render();
		virtual void renderShadowMap();
		virtual const BoundingSphere& getBoundingSphere() { return m_BoundingSphere; }
		virtual void update();
		virtual void prepareRender();
				
		void				setColor( const glm::vec3& v3Color )	{ m_v3Color = v3Color; }
		void				setIntensity( float fIntensity )		{ m_fIntensity = fIntensity; }
		void				setEnabled( bool bEnabled )				{ m_bEnabled = bEnabled; }
		
		ELightTpye			getLightType() const { return m_eLightType; }
		const glm::vec3&	getColor() const { return m_v3Color; }
		float				getIntensity() const { return m_fIntensity; }

		
		const Camera*		GetCamera() const { return &m_clLightViewCamera; }

		const glm::vec3&	GetPosition() const { return m_v3Position; }
		void				SetPosition( const glm::vec3& rPos ) { m_v3Position = rPos; }

protected:
		bool			m_bEnabled;
		glm::vec3		m_v3Color;
		float			m_fIntensity;
		ELightTpye		m_eLightType;
		BoundingSphere	m_BoundingSphere;
		Camera			m_clLightViewCamera;
		glm::vec3		m_v3Position;

		void		Init();
};

#endif