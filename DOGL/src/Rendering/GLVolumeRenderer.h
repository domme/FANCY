#ifndef GLVOLUMERENDERER_H
#define GLVOLUMERENDERER_H

#include "../Includes.h"
#include "ISceneRenderer.h"
#include "../Geometry/FullscreenQuad.h"
#include "../Scene/SceneManager.h"
#include "Materials/MAT_VolCube_Raycast_DSM.h"

class Engine;
class Mesh;
class Material;
class DirectionalLight;

class GLVolumeRenderer : public ISceneRenderer
{
public: 
	static GLVolumeRenderer& GetInstance() { static GLVolumeRenderer instance; return instance; }
	~GLVolumeRenderer();

	virtual void	Init( uint uScreenWidth, uint uScreenHeight, GLRenderer* pGLrenderer );
	virtual void	SetResolution( uint uScreenWidth, uint uScreenHeight );
	virtual GLuint	GetOutputTexture() const { return m_uFinalOutTex; }
	virtual GLuint	GetOutputFBO() const { return m_uFinalOutFBO; }
	virtual void	RenderScene( SceneManager* pScene );


protected:
	GLVolumeRenderer();
	void deleteResolutionDependentResources();
	void renderDeepShadowMap( const PointLight* pPointLight, const VolumeEntity* pVolumeEntity, SceneManager* pScene );

	MAT_VolCube_Raycast_DSM* m_pDSMmaterial;

	uint m_uDSM_width;
	uint m_uDSM_height;
	uint m_uDSM_depth;

	GLRenderer* m_pGLrenderer;
	Engine*		m_pEngine;
	FullscreenQuad* m_pFSquad;
	
	GLuint m_uScreenWidth;
	GLuint m_uScreenHeight;

	GLuint m_uFinalOutFBO;
	GLuint m_uFinalOutTex;

	GLuint m_uRayEndTex;
	GLuint m_uRayEndFBO;

	GLuint m_uRayEndTex_DSM;
	GLuint m_uRayEndFBO_DSM;

	GLuint m_uDepthStencilTex;

	GLuint m_uDeepShadowMapFBO;
	GLuint m_uDeepShadowMapTex;
	
};

#endif