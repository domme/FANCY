#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "../Includes.h"
#include "Shader.h"
#include "../Scene/Camera.h"
#include "UniformRegistry.h"
#include "../Light/Light.h"
#include "Managers/TextureManager.h"

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
	void SetResolution( uint uWidth, uint uHeight );

	bool	GetLightModeActive() const { return m_bActiveLightMode; }
	void	SetLightModeActive( bool bActive ) { m_bActiveLightMode = bActive; }
	//void	SetLightPass( ELightTpye eLightType ) { m_eActiveLightPass = eLightType; }

	void RenderMesh( const Mesh* pMesh, const glm::mat4& matModel, const glm::mat4& matWorld, const Camera* pCamera, const Material* pMaterial = NULL, Shader* pShader = NULL,  const Light* pLight = NULL );
	void prepareFrameRendering( const Camera* pCamera, const glm::mat4& matWorld );
	void prepareLightRendering( const Camera* pCamera, const Light* pLight );
	void prepareMeshRendering( const Mesh* pMesh, const glm::mat4& matModel, const glm::mat4& matWorld, const Camera* pCamera, const Material* pMaterial = NULL, Shader* pShader = NULL,  const Light* pLight = NULL );


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
	void setCurrLightIndex( uint uIdx )					{ m_uCurrLightIdx = uIdx; }
	uint getCurrLightIndex()							{ return m_uCurrLightIdx; }
	uint getStencilMask()								{ return m_uStencilMask; }
	void setStencilMask( uint uMask )					{ m_uStencilMask = uMask; }
	GLint getStencilRef()								{ return m_iStencilRef; }
	void setStencilRef( GLint iRef )					{ m_iStencilRef = iRef; } 
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