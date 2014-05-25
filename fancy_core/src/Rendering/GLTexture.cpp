#include "GLTexture.h"
#include "../IO/TextureLoader.h"
#include "Managers/GLResourceManager.h"
#include "Managers/TextureManager.h"

GLTexture::GLTexture() : 
m_uGlTextureLocation( GLUINT_HANDLE_INVALID ),
m_bInitialized( false ),
m_v3TextureSize( 0)
{

}

GLTexture::GLTexture( GLTexture& other ) :
m_uGlTextureLocation( GLUINT_HANDLE_INVALID ),
m_bInitialized( false )
{
	if( other.m_bInitialized )
		GLTextureResourceManager::GetInstance().AddResource( other.m_uGlTextureLocation );

	m_bInitialized = other.m_bInitialized;
	m_uGlTextureLocation = other.m_uGlTextureLocation;
	m_v3TextureSize = other.m_v3TextureSize;
}


GLTexture::~GLTexture()
{
	GLTextureResourceManager::GetInstance().HandleDelete( m_uGlTextureLocation );	
}

bool GLTexture::SetTexture1D( const String& szRelativeTexturePath )
{
	bool bSuccess = false;
	uint32 uTexture = TextureLoader::GetInstance().LoadTexture1D( szRelativeTexturePath, &bSuccess );

	if( !bSuccess )
		return false;

	GLTextureResourceManager::GetInstance().AddResource( uTexture );

	if( m_bInitialized )
		GLTextureResourceManager::GetInstance().HandleDelete( m_uGlTextureLocation );

	m_uGlTextureLocation = uTexture;

	m_bInitialized = true;

	return bSuccess;
}

void GLTexture::SetTexture( uint32 uGLtexLoc )
{
	GLTextureResourceManager::GetInstance().AddResource( uGLtexLoc );
	
	if( m_bInitialized )
		GLTextureResourceManager::GetInstance().HandleDelete( m_uGlTextureLocation );
	
	m_uGlTextureLocation = uGLtexLoc;

	const STextureInfo* pTexInfo = TextureInformationRegistry::GetInstance().GetInfoForTexture( uGLtexLoc );
	
	if( pTexInfo )
	{
		m_v3TextureSize.x = static_cast<float>(pTexInfo->m_uWidth);
		m_v3TextureSize.y = static_cast<float>(pTexInfo->m_uHeight);
		m_v3TextureSize.z = static_cast<float>(pTexInfo->m_uDepth);
	}
		
	m_bInitialized = true;
}

bool GLTexture::SetTexture( const String& szRelativeTexturePath )
{
	bool bSuccess = false;
	STextureInfo sInfo;
	uint32 uTexture = TextureLoader::GetInstance().LoadTexture2D( szRelativeTexturePath, &bSuccess, &sInfo );

	if( !bSuccess )
		return false;

	m_v3TextureSize.x = static_cast<float>(sInfo.m_uWidth);
	m_v3TextureSize.y = static_cast<float>(sInfo.m_uHeight);
	m_v3TextureSize.z = static_cast<float>(sInfo.m_uDepth);

	GLTextureResourceManager::GetInstance().AddResource( uTexture );
	
	if( m_bInitialized )
		GLTextureResourceManager::GetInstance().HandleDelete( m_uGlTextureLocation );
	
	m_uGlTextureLocation = uTexture;

	m_bInitialized = true;

	return bSuccess;
}