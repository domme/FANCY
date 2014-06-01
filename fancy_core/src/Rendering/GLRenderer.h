#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "../Includes.h"
#include "Shader.h"
#include "../Scene/Camera.h"
#include "UniformRegistry.h"
#include "../Light/Light.h"
#include "Managers/TextureManager.h"
#include "../Events/Events.h"

//#include "../Light/Light.h"

#include <map>

class Mesh;
class VolumeMesh;
class Material;

class DLLEXPORT GLRenderer
{
public:
	static GLRenderer& GetInstance() { static GLRenderer instance; return instance; }
	virtual ~GLRenderer();

	void InitMesh( Mesh* pMesh );
	void InitVolumeMesh( VolumeMesh* pVolumeMesh );
	void init( uint uWidth, uint uHeight );
	//void SetResolution( uint uWidth, uint uHeight );

	void OnResolutionChanged( glm::ivec2 iv2Resolution );
	//ListenerSingleParam<GLRenderer> GetResolutionChangedListener() { return ListenerSingleParam<GLRenderer>( this, OnResolutionChanged ); }

	bool	GetLightModeActive() const { return m_bActiveLightMode; }
	void	SetLightModeActive( bool bActive ) { m_bActiveLightMode = bActive; }
	//void	SetLightPass( ELightTpye eLightType ) { m_eActiveLightPass = eLightType; }

	void RenderMesh( Mesh* pMesh, const glm::mat4& matModelWorld, const Camera* pCamera, Material* pMaterial = NULL, Shader* pShader = NULL,  const Light* pLight = NULL );
	void prepareFrameRendering( const Camera* pCamera, const glm::mat4& matWorld );
	void prepareLightRendering( const Camera* pCamera, const Light* pLight );
	void prepareMeshRendering( Mesh* pMesh, const glm::mat4& matModelWorld, const Camera* pCamera, Material* pMaterial = NULL, Shader* pShader = NULL,  const Light* pLight = NULL );


	void setDepthTest( bool bEnable );
	void setDepthFunc( uint32 func );
	void setDepthMask( bool bMask ); 
	void setColorMask( bool bR, bool bG, bool bB, bool bA );
	void setStencilTest( bool bEnable );
	void setStencilFunc( uint32 func, int32 ref, uint32 mask );
	void setStencilOp( uint32 stencilFail, uint32 depthFail, uint32 depthPass );
	void setCulling( bool bEnable );
	void setCullFace( uint32 faceDir );
	void setBlending( bool bEnable );
	void setBlendFunc( uint32 src, uint32 dest );
	void setViewport( int x, int y, int width, int height );
	void setCurrLightIndex( uint uIdx )					{ m_uCurrLightIdx = uIdx; }
	uint getCurrLightIndex()							{ return m_uCurrLightIdx; }
	uint getStencilMask()								{ return m_uStencilMask; }
	void setStencilMask( uint uMask )					{ m_uStencilMask = uMask; }
	int32 getStencilRef()								{ return m_iStencilRef; }
	void setStencilRef( int32 iRef )					{ m_iStencilRef = iRef; } 
	uint getScreenWidth()								{ return m_uScreenWidth; }
	uint getScreenHeight()								{ return m_uScreenHeight; }
	void setAmbientLightColor( const glm::vec4& aCol )	{ m_v4AmbientLightColor = aCol; }
	const glm::vec4 getAmbientLightColor() const		{ return m_v4AmbientLightColor; }
	void setClearColor( const glm::vec4& cCol )			{ m_v4ClearColor = cCol; }
	const glm::vec4 getClearColor() const				{ return m_v4ClearColor; } 

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

	void saveViewport();
	void restoreViewport();
	
protected:
	GLRenderer();

	
	bool		m_bDepthTest;
	uint32		m_eDepthFunc;
	bool		m_bStencilTest;
	uint32		m_eStencilFunc;
	int32		m_iStencilRef;
	uint32		m_uStencilMask;
	uint32		m_eStencilOp_sFail;
	uint32		m_eStencilOp_depthFail;
	uint32		m_eStencilOp_depthPass;
	bool		m_bCulling;
	uint32		m_eCullFaceDir;
	bool		m_bBlending;
	uint32		m_eBlendSrc;
	uint32		m_eBlendDest;
	bool	m_bDepthMask;
	bool	m_bColorMask_r;
	bool	m_bColorMask_g;
	bool	m_bColorMask_b;
	bool	m_bColorMask_a;
	float				m_fHDRexposure;
	float				m_fHDRlightAdaption;
	bool				m_bShowDebugTextures;
	bool				m_bUseFXAA;
	bool				m_bUseToneMapping;
	bool				m_bUseBloom;


	uint	m_uScreenWidth;
	uint	m_uScreenHeight;
	glm::ivec4	m_v4Viewport;
	glm::ivec4	m_v4TempViewport;
	glm::vec4	m_v4AmbientLightColor;
	glm::vec4	m_v4ClearColor;
	

	uint		m_uCurrLightIdx;
	bool		m_bActiveLightMode; 

	TextureManager* m_pTextureManager;
	UniformRegistry* m_pUniformRegistry;
};

#endif