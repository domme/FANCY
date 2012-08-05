#include "GLResourcePathManager.h"
#include "TextureManager.h"

GLResourcePathManager::GLResourcePathManager()
{

}


GLResourcePathManager::~GLResourcePathManager()
{
	//NOTE: the actual GL-resource deletion is done in the ResourceManagers
	m_mapPathRes.clear();
}

bool GLResourcePathManager::HasResource( const String& szPath )
{
	return m_mapPathRes.count( szPath );
}

GLuint GLResourcePathManager::GetResource( const String& szPath )
{
	MapType::iterator iter = m_mapPathRes.find( szPath );

	if( iter == m_mapPathRes.end() )
		return 0;

	return iter->second;
}


void GLResourcePathManager::AddResource( const String& szPath, GLuint uResource )
{
	String szSearch = szPath;
	if( HasResource( szSearch ) )
		return;

	m_mapPathRes[ szSearch ] = uResource;
}

void GLResourcePathManager::HandleResourceDeletion( GLuint uResource )
{
	if( m_mapPathRes.size() == 0 )
		return;
	
	MapType::iterator iter;
	bool bFound = false;
	
	for( iter = m_mapPathRes.begin(); iter != m_mapPathRes.end(); ++iter )
	{
		if( iter->second == uResource )
		{
			bFound = true;
			break;
		}
	}

	if( bFound )
		m_mapPathRes.erase( iter );

	else
	{
		LOG( std::string( "WARNING: No Path-entry found for GL-Resource" ) );
		assert(false); //Break here
	}
}

//////////////////////////////////////////////////////////////////////////
//TexturePathManager

void GLTexturePathManager::HandleResourceDeletion( GLuint uResource )
{
	GLResourcePathManager::HandleResourceDeletion( uResource );
	
	TextureInformationRegistry::GetInstance().DeleteTexture( uResource );
}