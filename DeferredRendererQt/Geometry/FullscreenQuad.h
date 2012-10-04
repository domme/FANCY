#ifndef FULLSCREENQUAD_H
#define FULLSCREENQUAD_H

#include <includes.h>
#include <Rendering/Shader.h>
#include <Rendering/Materials/Material.h>

#include "../Rendering/Materials/MAT_FSquad_Textured.h"
#include "../Engine.h"

class FullscreenQuad
{
public:
	static FullscreenQuad&	getInstance();
	~FullscreenQuad();

	void					RenderTexture( GLuint uTexture );
	void					RenderWithMaterial( Material* pMat );

private:
	FullscreenQuad();
	void init();

	std::unique_ptr<Mesh>	m_pMesh;
	MAT_FSquad_Textured*	m_pMaterial;
	Engine*					m_pEngine;
};

#endif