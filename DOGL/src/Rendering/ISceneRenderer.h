#ifndef  ISCENERENDERER_H
#define ISCENERENDERER_H

#include "../Includes.h"
#include "../Scene/SceneManager.h"

class GLRenderer;

class ISceneRenderer
{
public:
	virtual GLuint	GetOutputTexture() const = 0;
	virtual GLuint	GetOutputFBO() const = 0;
	virtual void	Init( uint uScreenWidth, uint uScreenHeight, GLRenderer* pGLrenderer ) = 0;
	virtual void	SetResolution( uint uScreenWidth, uint uScreenHeight ) = 0;
	virtual void	RenderScene( SceneManager* pScene ) = 0;

};

#endif