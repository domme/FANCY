#ifndef ENGINE_H
#define ENGINE_H

#include <Includes.h>
#include <Rendering/TextureSemantics.h>

#include "Scene/SceneManager.h"
#include "Scene/CameraController.h"


//#include <assimp/aiAnim.h>

class Camera;
class Renderer;
class GLDeferredRenderer;

struct DLLEXPORT SDebugTexturePass
{
	SDebugTexturePass( TextureSemantics::eTexSemantic eSemantic ) { m_eTexSemantic = eSemantic; }
	TextureSemantics::eTexSemantic m_eTexSemantic;
};

class DLLEXPORT Engine
{
public:

	enum ERenderMode
	{
		RENDER_FORWARD,
		RENDER_DEFERRED,
		RENDER_DEPTH,
	};

	enum EVolumeMode
	{
		VOLUMES_SHOW_ONLY,
		VOLUMES_DONT_SHOW,
		VOLUMES_SHOW_BOTH,
	};

	


	static Engine&	GetInstance();

	virtual void			Update( const uint elapsedTicksMS );
	virtual void			Render( const uint elapsedTicksMS );
	void					Init(  uint uScreenWidth, uint uScreenHeight ,const glm::vec4& v4AmbientColor, const glm::vec4& v4ClearColor, float fClearDepth );
	glm::mat4*				GetWorldMat();
	
	GLRenderer*				GetRenderer() { return m_pRenderer; }
	float					GetFPS();
	const glm::vec4&		GetAmbientLightColor() const { return m_v4AmbientColor; }
	void					SetAmbientLightColor( const glm::vec4& v4Ambient ) { m_v4AmbientColor = v4Ambient; }

	const glm::vec4&		GetClearColor() const { return m_v4ClearColor; }
	void					SetClearColor( const glm::vec4& v4Clear ) { m_v4ClearColor = v4Clear; }

	SceneManager*			GetScene() { return m_pScene; }
	void					SetScene( SceneManager* pScene ) { m_pScene = pScene; }



	float					GetClearDepth() const { return m_fClearDepth; }
	void					SetClearDepth( float d ) { m_fClearDepth = d; }

	void					SetResolution( uint uWidth, uint uHeight );
	uint					GetScreenWidth() const { return m_uScreenWidth; }
	uint					GetScreenHeight() const { return m_uScreenHeight; }
	uint					GetElapsedTime() const { return m_uAbsElapsedTicksMS; }
	float					GetMovementMul() const { return (float) m_uDeltaTicksMS / 1000.0f; }

	ERenderMode				getRenderMode() const { return m_eRenderMode; }
	void					setRenderMode( ERenderMode eMode ) { m_eRenderMode = eMode; }
	uint					getNextMeshID() { return m_uNumMeshes++; }

	const std::vector<SDebugTexturePass>& GetDebugTexturePasses() { return m_vDebugTexturePasses; }
	void AddDebugTexturePass( TextureSemantics::eTexSemantic eSemantic ) { m_vDebugTexturePasses.push_back( SDebugTexturePass( eSemantic ) ); }

	

	void					SetVolumeMode( EVolumeMode eVolMode ) { m_eVolumeMode = eVolMode; }
	EVolumeMode				GetVolumeMode() const { return m_eVolumeMode; } 

//	template<typename ObjectT>
	//int						AddResolutionListener( ObjectT* pListener, void (ObjectT::*callbackFunc) (glm::vec2) );
	

private:
	Engine();
	~Engine();	

	glm::mat4			m_gMatWorld;
	SceneManager*		m_pScene;
	GLRenderer*			m_pRenderer;

	float				m_fCurrentFPS;
	glm::vec4			m_v4AmbientColor;
	glm::vec4			m_v4ClearColor;
	float				m_fClearDepth;
	
	uint				m_uCurrentElapsedTicksMS;
	uint				m_uAbsElapsedTicksMS;
	uint				m_uDeltaTicksMS;
	uint				m_uCurrentFrameCount;
	uint				m_uScreenHeight;
	uint				m_uScreenWidth;
	void				UpdateFPS( const uint elapsedTicksMS );
	ERenderMode			m_eRenderMode;
	uint				m_uNumMeshes;

	EVolumeMode			m_eVolumeMode;
	GLDeferredRenderer* m_pDeferredRenderer;
	
	bool				m_bInitialized;

	std::vector<SDebugTexturePass> m_vDebugTexturePasses;

//	DelegateSingleParam<glm::ivec2> m_delegateResolutionChanged;

};




#endif