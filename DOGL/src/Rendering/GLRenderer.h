#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "../Includes.h"
#include "Shader.h"
#include "../Geometry/FullscreenQuad.h"
#include "../Scene/SceneManager.h"
#include "../Scene/Camera.h"
#include "ISceneRenderer.h"
#include "UniformRegistry.h"

#include "GLDeferredRenderer.h"
//#include "../Light/Light.h"

#include <string>
#include <map>


class TransformSQT;
class Engine;

class Mesh;
class VolumeMesh;
class Material;

class MAT_Pointlight_Illumination;
class MAT_FSquad_FXAA;
class MAT_FSquad_ToneMapping;
class MAT_FSquad_LumaTimeAdaption;
class MAT_FSquad_GaussianBlur;
class MAT_FSquad_Bloom;
class MAT_FSquad_BrightPass;

class DLLEXPORT GLRenderer
{
	//befriend with Pass-classes, that need to access the core rendering functionalities
	friend class Pass_GaussianBlur;
	friend class GLDeferredRenderer;
	friend class GLVolumeRenderer;

public:
	static GLRenderer& GetInstance() { static GLRenderer instance; return instance; }
	virtual ~GLRenderer();

	void InitMesh( Mesh* pMesh );
	void InitVolumeMesh( VolumeMesh* pVolumeMesh );
	void RenderScene(  SceneManager* pSceneManager );
	void init( uint uWidth, uint uHeight );
	void SetResolution( uint uWidth, uint uHeight );

	bool	GetLightModeActive() const { return m_bActiveLightMode; }
	void	SetLightModeActive( bool bActive ) { m_bActiveLightMode = bActive; }
	//void	SetLightPass( ELightTpye eLightType ) { m_eActiveLightPass = eLightType; }

	void RenderMesh( const Mesh* pMesh, Shader* pShader, const Camera* pCamera, const SceneManager* pScene, const glm::mat4& matModelWorld, const Material* pMaterial = NULL );
				
protected:
	GLRenderer();

	void prepareFrameRendering( const Camera* pCamera, const SceneManager* pScene );
	void prepareLightRendering( const Camera* pCamera, const SceneManager* pScene );
	void prepareMeshRendering( const Mesh* pMesh, Shader* pShader, const Camera* pCamera, const SceneManager* pScene, const glm::mat4& matModelWorld, const Material* pMaterial ); 
	

	void deleteResolutionDependentResources();
	void updateTextures();
	void updatePostproMaterials();

	void setDepthTest( bool bEnable );
	void setDepthFunc( GLenum func );
	void setDepthMask( GLboolean bMask ); 
	void setColorMask( bool bR, bool bG, bool bB, bool bA );
	void setStencilTest( bool bEnable );
	void setStencilFunc( GLenum func, GLint ref, GLuint mask );
	void setStencilOp( GLenum stencilFail, GLenum depthFail, GLenum depthPass );
	void setCulling( bool bEnable );
	void setCullFace( GLenum faceDir );
	void setBlending( bool bEnable );
	void setBlendFunc( GLenum src, GLenum dest );
	void setViewport( int x, int y, int width, int height );

	void saveViewport();
	void restoreViewport();
	
	bool		m_bDepthTest;
	GLenum		m_eDepthFunc;
	bool		m_bStencilTest;
	GLenum		m_eStencilFunc;
	GLint		m_iStencilRef;
	GLuint		m_uStencilMask;
	GLenum		m_eStencilOp_sFail;
	GLenum		m_eStencilOp_depthFail;
	GLenum		m_eStencilOp_depthPass;
	bool		m_bCulling;
	GLenum		m_eCullFaceDir;
	bool		m_bBlending;
	GLenum		m_eBlendSrc;
	GLenum		m_eBlendDest;
	GLboolean	m_bDepthMask;
	GLboolean	m_bColorMask_r;
	GLboolean	m_bColorMask_g;
	GLboolean	m_bColorMask_b;
	GLboolean	m_bColorMask_a;

	uint	m_uScreenWidth;
	uint	m_uScreenHeight;
	glm::vec4	m_v4Viewport;
	glm::vec4	m_v4TempViewport;

	Engine* m_pEngine;

	uint		m_uCurrLightIdx;
	bool		m_bActiveLightMode; 

	FullscreenQuad* m_pFSquad;
	TextureManager* m_pTextureManager;
	UniformRegistry* m_pUniformRegistry;


	//////////////////////////////////////////////////////////////////////////
	//Render-Modules
	ISceneRenderer* m_pSceneRenderer;
	ISceneRenderer* m_pVolumeRenderer;


	//////////////////////////////////////////////////////////////////////////
	//Materials
	
	//Tone-mapping stuff
	/////////////////////////////////////////////////////////////////////////
	MAT_FSquad_ToneMapping*			m_pMAT_ToneMap;
	MAT_FSquad_LumaTimeAdaption*	m_pMAT_LumaTimeAdaption;


	//Render Passes
	//////////////////////////////////////////////////////////////////////////
	Pass_GaussianBlur* m_pPassGaussianBlur;
		
	
	//Postpro-Materials
	//////////////////////////////////////////////////////////////////////////
	MAT_FSquad_FXAA*				m_pMAT_FXAA;

	MAT_FSquad_GaussianBlur*		m_pMAT_GaussianBlur;
	MAT_FSquad_BrightPass*			m_pMAT_BrightPass;
	MAT_FSquad_Bloom*				m_pMAT_Bloom;

	
	//////////////////////////////////////////////////////////////////////////
	// Textures and FBOs
	
	union {
		GLuint m_uSmall_RGB32F_FBO_01;
		GLuint m_uBloomFBO_01;
	};

	union {
		GLuint m_uSmall_RGB32F_FBO_02;
		GLuint m_uBrightPassFBO_02;
	};

	union {
		GLuint m_uSmall_LUM32F_FBO_03;
		GLuint m_uFinalLuminanceFBO_03;
	};

	union {
		GLuint m_uSmall_LUM32F_FBO_04;
		GLuint m_uCurrLuminanceFBO_04;
	};

	union {
		GLuint m_uSmall_LUM32F_FBO_05;
		GLuint m_uLastLuminanceFBO_05;
	};

	union {
		GLuint m_uRGB32F_FBO_08;
		GLuint m_uFinalBeforeTonemapFBO_08;
	};

	union {
		GLuint m_uRGB32F_FBO_09;
		GLuint m_uFinalTonemapFBO_09;
	};


	union {
		GLuint m_uSmall_RGB32F_Tex_01;
		GLuint m_uBloomTex_01;
	};

	union {
		GLuint m_uSmall_RGB32F_Tex_02;
		GLuint m_uBrightPassTex_02;
	};

	union {
		GLuint m_uSmall_LUM32F_Tex_03;
		GLuint m_uFinalLuminanceTex_03;
	};

	union {
		GLuint m_uSmall_LUM32F_Tex_04;
		GLuint m_uCurrLuminanceTex_04;
	};

	union {
		GLuint m_uSmall_LUM32F_Tex_05;
		GLuint m_uLastLuminanceTex_05;
	};

	union {
		GLuint m_uRGB32F_Tex_08;
		GLuint m_uFinalBeforeTonemapTex_08;
	};

	union {
		GLuint m_uRGB32F_Tex_09;
		GLuint m_uFinalTonemappedTex_09;
	};
	
};

#endif