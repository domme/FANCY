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
		virtual void		PrepareShadowmapPass( int uPassIndex );
		virtual void		PostprocessShadowmap();

		float				getFalloffStart() const { return m_fFalloffStart; }
		void				setFalloffStart( float fFalloffStart ) { m_fFalloffStart = fFalloffStart; }

		float				getFalloffEnd() const { return m_fFalloffEnd; }
		void				setFalloffEnd( float fFalloffEnd ) { m_fFalloffEnd = fFalloffEnd; }


		uint32				GetShdowCubeMap() { return m_uShadowCubeDepthTex; }
		uint32				GetDebugTex() { return m_uDebugTex; }
		
		
	protected:
		
		float				m_fFalloffStart;
		float				m_fFalloffEnd;


		uint32				m_uShadowCubeDepthTex;
		
		//uint32				m_uShadowFBOs[ 6 ];
		//uint32				m_uShadowDepthTextures[ 6 ];
		//uint32				m_uShadowColorTextures[ 6 ];

		glm::quat			m_quatShadowCamOrientations[ 6 ];


		//DEBUG:
		uint32				m_uDebugFBO;
		uint32				m_uDebugTex;

		virtual	void		initShadowmap();
		virtual void		destroyShadowmap();
		virtual void		onPositionChanged();

		void				initShadowCamera();
};

#endif