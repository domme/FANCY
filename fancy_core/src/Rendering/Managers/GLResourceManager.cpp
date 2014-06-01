#include "GLResourceManager.h"

#include "GLResourcePathManager.h"


GLResourceManager::GLResourceManager()
{

}

GLResourceManager::~GLResourceManager()
{
	for( std::map<uint32, uint>::iterator iter = m_mapResRef.begin(); iter != m_mapResRef.end(); ++iter )
	{
		deleteResource( iter->first );
	}

	m_mapResRef.clear();
}

bool GLResourceManager::HasResource( uint32 uglResource )
{
	return m_mapResRef.find( uglResource ) != m_mapResRef.end();
}

void GLResourceManager::HandleDelete( uint32 uglResource )
{
	if( m_mapResRef.size() == 0 )
		return;
	
	MapType::iterator iter = m_mapResRef.find( uglResource );

	if( iter == m_mapResRef.end() )
		return;

	--iter->second;

	if( iter->second <= 0 )
	{
		deleteResource( uglResource );
		m_mapResRef.erase( iter );

		std::stringstream ss;
		ss << "Deleting \t\t\t" << getDebugMgrName() << uglResource;
		LOG( ss.str() );
	}

	else
	{
		std::stringstream ss;
		ss << "Decreasing \t\t" << getDebugMgrName() << uglResource << " - Reference-count is now: " << m_mapResRef[ uglResource ];
		LOG( ss.str() );
	}

}

void GLResourceManager::AddResource( uint32 uglResource )
{
	MapType::iterator iter = m_mapResRef.find( uglResource );

	if( iter != m_mapResRef.end() )
		++iter->second;

	else
		m_mapResRef[ uglResource ] = 1;

	std::stringstream ss;
	ss << "Adding \t\t\t" << getDebugMgrName() << uglResource << " - Reference-count is now: " << m_mapResRef[ uglResource ];
	LOG( ss.str() );
}

int GLResourceManager::GetResourceUseCount( uint32 uglResource )
{
	MapType::iterator iter = m_mapResRef.find( uglResource );

	if( iter == m_mapResRef.end() )
		return -1;

	return iter->second;
}

void GLResourceManager::deleteResource( uint32 uglResource )
{
	//Dummy in base class
}



//////////////////////////////////////////////////////////////////////////
//GLTextureResourceManager
//////////////////////////////////////////////////////////////////////////
void GLTextureResourceManager::deleteResource( uint32 uglResource )
{
	glDeleteTextures( 1, &uglResource );
	GLTexturePathManager::GetInstance().HandleResourceDeletion( uglResource );
}


//////////////////////////////////////////////////////////////////////////
//GLVBOResourceManager
//////////////////////////////////////////////////////////////////////////
void GLVBOResourceManager::deleteResource( uint32 uglResource )
{
	glDeleteBuffers( 1, &uglResource );
	GLVBOpathManager::GetInstance().HandleResourceDeletion( uglResource );
}


//////////////////////////////////////////////////////////////////////////
//GLIBOResourceManager
//////////////////////////////////////////////////////////////////////////
void GLIBOResourceManager::deleteResource( uint32 uglResource )
{
	glDeleteBuffers( 1, &uglResource );
	GLIBOpathManager::GetInstance().HandleResourceDeletion( uglResource );
}


//////////////////////////////////////////////////////////////////////////
//GLProgramResourceManager
//////////////////////////////////////////////////////////////////////////
void GLProgramResourceManager::deleteResource( uint32 uglResource )
{
	glDeleteProgram( uglResource );
	GLShaderPathManager::GetInstance().HandleResourceDeletion( uglResource );
}