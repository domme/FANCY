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
		void				setPositionWS( const glm::vec3 v3Pos )  { m_mat4TransformWS[ 3 ] = glm::vec4( v3Pos, 1.0f ); }
		glm::vec3			getPositionWS()	const					{ return glm::vec3( m_mat4TransformWS[ 3 ].x, m_mat4TransformWS[ 3 ].y, m_mat4TransformWS[ 3 ].z ); } 

		ELightTpye			getLightType() const { return m_eLightType; }
		const glm::vec3&	getColor() const { return m_v3Color; }
		float				getIntensity() const { return m_fIntensity; }

		
		const Camera*		GetCamera() const { return &m_clLightViewCamera; }

		const glm::mat4		getTransformWS() const { return m_mat4TransformWS; }

protected:
		bool			m_bEnabled;
		glm::vec3		m_v3Color;
		float			m_fIntensity;
		ELightTpye		m_eLightType;
		BoundingSphere	m_BoundingSphere;
		Camera			m_clLightViewCamera;

		glm::mat4		m_mat4TransformWS;
		
		void		Init();
};

#endif