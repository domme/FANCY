#ifndef FULLSCREENQUAD_H
#define FULLSCREENQUAD_H

#include "../includes.h"
//#include "OpenGL_Includes.h"
#include "../Rendering/Shader.h"
#include "../Rendering/Materials/MAT_FSquad_Textured.h"
#include "../Rendering/Materials/MAT_FSquad_Textured3D.h"
#include "../Engine.h"

class FullscreenQuad
{
public:
	static FullscreenQuad& getInstance();
	~FullscreenQuad();

	void RenderTexture( GLuint uTexture );
	void RenderTimeTexture3D( GLuint uTexture );
	void RenderWithMaterial( Material* pMat );

private:
	FullscreenQuad();
	void init();

	std::unique_ptr<Mesh> m_pMesh;
	MAT_FSquad_Textured* m_pMaterial;
	MAT_FSquad_Textured3D* m_pMatTex3D;
	Engine* m_pEngine;
};

#endif