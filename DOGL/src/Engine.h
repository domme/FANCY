#ifndef ENGINE_H
#define ENGINE_H

#include "includes.h"
#include "Rendering/TextureSemantics.h"

//#include <assimp/aiAnim.h>

class Camera;
class GLRenderer;

class DLLEXPORT Engine
{
public:
		

	static Engine&	GetInstance();

	virtual void			Update( const uint elapsedTicksMS );
	virtual void			Render( const uint elapsedTicksMS );
	void					Init(  uint uScreenWidth, uint uScreenHeight ,const glm::vec4& v4AmbientColor, const glm::vec4& v4ClearColor, float fClearDepth );
	glm::mat4*				GetWorldMat();
	Camera*					GetCurrentCamera();
	void					SetCurrentCamera( Camera* pCam ) { m_pRenderCamera = pCam; } 

	GLRenderer*				GetRenderer() { return m_pRenderer; }
	float					GetFPS();
	const glm::vec4&		GetAmbientLightColor() const { return m_v4AmbientColor; }
	void					SetAmbientLightColor( const glm::vec4& v4Ambient ) { m_v4AmbientColor = v4Ambient; }

	const glm::vec4&		GetClearColor() const { return m_v4ClearColor; }
	void					SetClearColor( const glm::vec4& v4Clear ) { m_v4ClearColor = v4Clear; }
		

	float					GetClearDepth() const { return m_fClearDepth; }
	void					SetClearDepth( float d ) { m_fClearDepth = d; }

	void					SetResolution( uint uWidth, uint uHeight );
	uint					GetScreenWidth() const { return m_uScreenWidth; }
	uint					GetScreenHeight() const { return m_uScreenHeight; }
	uint					GetElapsedTime() const { return m_uAbsElapsedTicksMS; }
	float					GetMovementMul() const { return (float) m_uDeltaTicksMS / 1000.0f; }
	
	uint					getNextMeshID() { return m_uNumMeshes++; }

	void					SetDebugTexturesVisible( bool bVisible ) { m_bShowDebugTextures = bVisible; }
	bool					GetDebugTexturesVisible() const { return m_bShowDebugTextures; }

	void					SetFXAAenabled( bool bEnable ) { m_bUseFXAA = bEnable; }
	bool					GetFXAAenabled() const { return m_bUseFXAA; }

	void					SetHDRexposure( float fexp ) { m_fHDRexposure = fexp; } 
	float					GetHDRExposure() const { return m_fHDRexposure; }

	void					SetToneMappingEnabled( bool bEnable ) { m_bUseToneMapping = bEnable; }
	bool					GetToneMappingEnabled() const { return m_bUseToneMapping; }

	void					SetHDRlightAdaption( float fAdapt ) { m_fHDRlightAdaption = fAdapt; }
	float					GetHDRlightAdaption() const { return m_fHDRlightAdaption; }

	void					SetUseBloom( bool bBloom ) { m_bUseBloom = bBloom; } 
	bool					GetUseBloom() const { return m_bUseBloom; }

		

	template<typename ObjectT>
	int RegisterKeyDownListener( ObjectT* pListener, void (ObjectT::*callbackFunc) (uint16) )
	{
		//TODO: Reimplement!
		return -1;
	}

	static bool UnregisterKeyDownListener( int id )
	{
		//TODO: Reimplement!
		return false;
	}


	template<typename ObjectT>
	int RegisterKeyClickedListener( ObjectT* pListener, void (ObjectT::*callbackFunc) (uint16) )
	{
		//TODO: Reimplement!
		return -1;
	}

	static bool UnregisterKeyClickedListener( int id )
	{
		//TODO: Reimplement!
		return false;
	}

private:
	Engine();
	~Engine();	

	glm::mat4			m_gMatWorld;
	Camera*				m_pRenderCamera;
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
	uint				m_uNumMeshes;
	
	float				m_fHDRexposure;
	float				m_fHDRlightAdaption;

	bool				m_bInitialized;
	bool				m_bShowDebugTextures;
	bool				m_bUseFXAA;
	bool				m_bUseToneMapping;
	bool				m_bUseBloom;


};

#endif