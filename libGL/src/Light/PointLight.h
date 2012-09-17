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
		virtual void		update();
	
		float				getFalloffStart() const { return m_fFalloffStart; }
		void				setFalloffStart( float fFalloffStart ) { m_fFalloffStart = fFalloffStart; }

		float				getFalloffEnd() const { return m_fFalloffEnd; }
		void				setFalloffEnd( float fFalloffEnd ) { m_fFalloffEnd = fFalloffEnd; }

		
		
	protected:
		
		float				m_fFalloffStart;
		float				m_fFalloffEnd;

};

#endif