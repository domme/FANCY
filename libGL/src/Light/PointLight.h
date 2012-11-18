#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "../includes.h"
#include "Light.h"

class DLLEXPORT PointLight : public Light
{
	friend class GLVolumeRenderer;

	public:
							PointLight();
		virtual				~PointLight();
		virtual void		Update();	
		virtual void		Init();
		virtual void		PrepareShadowmapPass( uint uPassIndex );
		virtual void		PostprocessShadowmap();

		float				getFalloffStart() const { return m_fFalloffStart; }
		void				setFalloffStart( float fFalloffStart ) { m_fFalloffStart = fFalloffStart; }

		float				getFalloffEnd() const { return m_fFalloffEnd; }
		void				setFalloffEnd( float fFalloffEnd ) { m_fFalloffEnd = fFalloffEnd; }

		GLuint				GetShdowCubeMap() { return m_uShadowCubeDepthTex; }
		
		
	protected:
		
		float				m_fFalloffStart;
		float				m_fFalloffEnd;


		GLuint				m_uShadowCubeDepthTex;
		GLuint				m_uShadowCubeFBO;

		//GLuint				m_uShadowFBOs[ 6 ];
		//GLuint				m_uShadowDepthTextures[ 6 ];
		//GLuint				m_uShadowColorTextures[ 6 ];

		glm::quat			m_quatShadowCamOrientations[ 6 ];

		virtual	void		initShadowmap();
		virtual void		destroyShadowmap();

		void				initShadowCamera();
};

#endif