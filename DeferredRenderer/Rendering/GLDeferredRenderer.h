#ifndef GLDEFERREDRENDERER_H
#define GLDEFERREDRENDERER_H

#include <Includes.h>
#include <Rendering/Shader.h>
#include <Scene/Camera.h>
#include <Rendering/GLTexture.h>
#include <Light/DirectionalLight.h>
#include <Light/SpotLight.h>

#include "../Geometry/FullscreenQuad.h"
#include "../Scene/SceneManager.h"

class Engine;
class Mesh;
class Material;

class MAT_FSquad_FinalComposite;
class MAT_FSquad_DirLighting;
class MAT_FSquad_PointLighting;
class MAT_Pointlight_Illumination;
class MAT_FSquad_FXAA;
class MAT_FSquad_ToneMapping;
class MAT_FSquad_LumaTimeAdaption;
class MAT_FSquad_GaussianBlur;
class MAT_FSquad_Bloom;
class MAT_FSquad_BrightPass;
class GLRenderer;
class Pass_GaussianBlur;


class TextureManager;

namespace GBuffer
{
	enum E_GBuffer
	{
		ColorGloss = 0,
		Spec,
		Normal,
		Depth,

		num
	};
}

class GLDeferredRenderer
{
	public: 
		static GLDeferredRenderer& GetInstance();
		
		GLuint GetOutputTexture() const { return m_uFinalCompositeTex_07; }
		GLuint GetOutputFBO() const { return m_uFinalCompositeFBO_07; }
		void Init( uint uWidth, uint uHeight, GLRenderer* glRenderer );
		void SetResolution( uint uWidth, uint uHeight );
		void RenderScene( SceneManager* pSceneManager );

	protected:
		GLDeferredRenderer();
		~GLDeferredRenderer();

	private:
		uint m_uScreenWidth;
		uint m_uScreenHeight;


		FullscreenQuad* m_pFSquad;
		

		void updateTextures();
		void updatePostproMaterials();
		void renderEntities( SceneManager* pScene, Camera* pCamera );
		void renderEntity( Entity* pEntity, const SceneManager* pScene, const Camera* pCamera );
		void renderDirLight( DirectionalLight* pLight, SceneManager* pScene, Camera* pCamera );
		void renderPointLight( PointLight* pLight, SceneManager* pScene, Camera* pCamera );
		void renderShadowMap( PointLight* pLight, SceneManager* pScene, Camera* pCamera ); 
		void renderShadowMap( DirectionalLight* pLight, SceneManager* pScene, Camera* pCamera );


		void deleteResoultionDependentResources();



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
		//FBOs
		//////////////////////////////////////////////////////////////////////////
		GLuint m_uDeferredFBO;
		
		
		union {
			GLuint m_uRGB32F_FBO_06;
			GLuint m_uLightingFBO_06;
		};

		union {
			GLuint m_uRGB32F_FBO_07;
			GLuint m_uFinalCompositeFBO_07;
		};
				
		//////////////////////////////////////////////////////////////////////////
			

		union {
			GLuint m_uRGB32F_Tex_06;
			GLuint m_uLightingTex_06;
		};

		union {
			GLuint m_uRGB32F_Tex_07;
			GLuint m_uFinalCompositeTex_07;
		};


		//From old GLRenderer:
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

		//////////////////////////////////////////////////////////////////////////


		GLuint m_uDeferredDepthStencilTex;
		GLuint m_uGBuffer[ GBuffer::num ];
		
								
		Engine* m_pEngine;
		GLRenderer* m_pGLrenderer;
		TextureManager* m_pTextureManager;

		
		//Light-Materials
		//////////////////////////////////////////////////////////////////////////
		MAT_FSquad_DirLighting*			m_pMAT_Dirlight;
		MAT_FSquad_PointLighting*		m_pMAT_Pointlight;

		//Postpro-Materials
		//////////////////////////////////////////////////////////////////////////
		MAT_FSquad_FinalComposite*		m_pMAT_FinalComposite;
		
		//Helper-Meshes
		Mesh*							m_pPointlightMesh;

};

#endif