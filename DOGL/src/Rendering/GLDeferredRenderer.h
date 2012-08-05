#ifndef GLDEFERREDRENDERER_H
#define GLDEFERREDRENDERER_H

#include "../includes.h"
#include "Shader.h"
#include "../Geometry/FullscreenQuad.h"
#include "../Scene/SceneManager.h"
#include "../Scene/Camera.h"
#include "GLTexture.h"
#include "../Light/DirectionalLight.h"
#include "../Light/SpotLight.h"
#include "ISceneRenderer.h"


class Engine;
class Mesh;
class Material;



class MAT_FSquad_FinalComposite;
class MAT_FSquad_DirLighting;
class MAT_FSquad_PointLighting;
class MAT_Pointlight_Illumination;
class GLRenderer;


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

class GLDeferredRenderer : public ISceneRenderer
{
	public: 
		static GLDeferredRenderer& GetInstance();
		
		virtual GLuint GetOutputTexture() const { return m_uFinalCompositeTex_07; }
		virtual GLuint GetOutputFBO() const { return m_uFinalCompositeFBO_07; }
		virtual void Init( uint uWidth, uint uHeight, GLRenderer* glRenderer );
		virtual void SetResolution( uint uWidth, uint uHeight );
		virtual void RenderScene( SceneManager* pSceneManager );

	protected:
		GLDeferredRenderer();
		virtual ~GLDeferredRenderer();

	private:
		uint m_uScreenWidth;
		uint m_uScreenHeight;


		FullscreenQuad* m_pFSquad;
		

		void updateTextures();
		void renderEntity( const Entity* pEntity, const SceneManager* pScene, const Camera* pCamera );
		void renderDirLight( DirectionalLight* pLight, const SceneManager* pScene, const Camera* pCamera );
		void renderPointLight( PointLight* pLight, const SceneManager* pScene, const Camera* pCamera );


		void deleteResoultionDependentResources();

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
		unique_ptr<Mesh> m_pPointlightMesh;

};

#endif