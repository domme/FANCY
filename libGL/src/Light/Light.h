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

		virtual void					Init();
		virtual const BoundingSphere&	GetBoundingSphere() { return m_BoundingSphere; }
		virtual void					Update();
		virtual void					PrepareShadowmapPass( uint uPassIndex );
		virtual void					PostprocessShadowmap();

		uint							GetNumShadowmapPasses() { return m_uNumShadowmapPasses; }
		

		void							SetColor( const glm::vec3& v3Color )	{ m_v3Color = v3Color; }
		void							SetIntensity( float fIntensity )		{ m_fIntensity = fIntensity; }
		void							SetEnabled( bool bEnabled )				{ m_bEnabled = bEnabled; }
		bool							GetEnabled() const						{ return m_bEnabled; }

		ELightTpye						GetLightType() const { return m_eLightType; }
		const glm::vec3&				GetColor() const { return m_v3Color; }
		float							GetIntensity() const { return m_fIntensity; }

		
		Camera*							GetCamera() { return &m_clLightViewCamera; }
		const Camera*					GetCamera() const { return &m_clLightViewCamera; }


		const glm::vec3&				GetPosition() const { return m_v3Position; }
		void							SetPosition( const glm::vec3& rPos ) { m_v3Position = rPos; }

		bool							GetCastShadows() const { return m_bCastShadows; }
		void							SetCastShadows( bool bCastShadows ) { m_bCastShadows = bCastShadows; }

		const glm::ivec2&				GetShadowmapResolution() const { return m_iv2ShadowmapResolution; }
		void							SetShadowmapResolution( const glm::ivec2& v2Res );

		void							SetDirty( bool bDirty ) { m_bDirty = bDirty; }
		bool							GetDirty() { return m_bDirty; }

		
protected:
		bool			m_bDirty;
		bool			m_bEnabled;
		bool			m_bShadowmapInitialized;
		bool			m_bCastShadows;
		ELightTpye		m_eLightType;
		float			m_fIntensity;
		glm::ivec2		m_iv2ShadowmapResolution;
		glm::vec3		m_v3Color;
		glm::vec3		m_v3Position;
		BoundingSphere	m_BoundingSphere;
		Camera			m_clLightViewCamera;
		uint			m_uNumShadowmapPasses;


		virtual void					initShadowmap();
		virtual void					destroyShadowmap();

		
};

#endif