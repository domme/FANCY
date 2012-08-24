#ifndef LIGHT_H
#define LIGHT_H

#include "../includes.h"
#include "../Geometry/Mesh.h"
#include "../Scene/Camera.h"
//#include "OpenGL_Includes.h"
#include "../Rendering/Shader.h"
#include "../Rendering/Materials/Material.h"

class Engine;

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
		virtual void onTransformChanged( const glm::mat4& newTransform );
		
		void				setColor( const glm::vec3& v3Color )	{ m_v3Color = v3Color; }
		void				setIntensity( float fIntensity )		{ m_fIntensity = fIntensity; }
		void				setEnabled( bool bEnabled )				{ m_bEnabled = bEnabled; }
		void				setChachedPosition( glm::vec3 vPos )	{ m_vChachedPosition = vPos; }

		ELightTpye			getLightType() const { return m_eLightType; }
		const glm::vec3&	getColor() const { return m_v3Color; }
		float				getIntensity() const { return m_fIntensity; }
		const glm::vec3&	getCachedPosition() const { return m_vChachedPosition; }

		const Camera*		GetCamera() const { return &m_clLightViewCamera; }

protected:
		bool			m_bEnabled;
		glm::vec3		m_v3Color;
		float			m_fIntensity;
		ELightTpye		m_eLightType;
		glm::vec3		m_vChachedPosition;
		BoundingSphere	m_BoundingSphere;
		Engine*			m_pEngine;
		Camera			m_clLightViewCamera;
		
		void		Init();
};

#endif