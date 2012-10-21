#include "PathService.h"
#include <stdio.h>

#ifdef __WINDOWS
#include <Windows.h>
#endif

#ifdef __OSX
#include <mach-o/dyld.h>
#endif

std::string PathService::m_szRelativeResourcePath;

String PathService::getExePath()
{
	String pathOut = "";

#ifdef __WINDOWS
	TCHAR outString[FILENAME_MAX];
	int bytes = GetModuleFileName( NULL, outString, FILENAME_MAX );

	pathOut = outString;
	removeAppName( pathOut );
#endif

#ifdef __OSX
#endif

	return pathOut;
}

String PathService::GetContainingFolder( const String& szFileName )
{
	String szFolderPath = szFileName;
	removeAppName( szFolderPath );
	return szFolderPath;
}

String PathService::convertToAbsPath( const String& szRelPath, bool bInResources /* = true */ )
{
	if( bInResources )
		return getResourcesPath() + szRelPath;

	else
		return getExePath() + szRelPath;
}

void PathService::convertToAbsPath( String& szRelPath, bool bInResources /* = true */ )
{
	if( bInResources )
		szRelPath = getResourcesPath() + szRelPath;

	else
		szRelPath = getExePath() + szRelPath;
}

void PathService::SetResourceLocation( const String& szResource )
{
	m_szRelativeResourcePath = szResource;
}

String PathService::getResourcesPath()
{
	static String szPath = "";

	if( szPath != "" )
	{
		return szPath;
	}

#ifdef __WINDOWS
	szPath = getExePath();

	if( m_szRelativeResourcePath.size() < 1 )
		szPath += "\\Resources\\";

	else
		szPath += m_szRelativeResourcePath;

#else
    char szModulePath[2048];
    uint uCount = sizeof( szModulePath );
    _NSGetExecutablePath( szModulePath, &uCount );
    
    String szTempPath ( szModulePath );
    
    for( int i = 0; i < 2; ++i )
	{
		int iPos = szTempPath.find_last_of( "/" );
		szTempPath = szTempPath.substr( 0, iPos );
	}
    
    szPath.append( szTempPath );
    szPath += "/Resources/";
    fprintf( stdout, "%s", szPath.c_str() );
   
#endif

	return szPath;
}

String PathService::getFileType( const String& szFileName )
{
	int iPos = szFileName.find_last_of( "." );
	return szFileName.substr( iPos + 1, szFileName.size() - iPos );
}


void PathService::removeAppName( String& szPath )
{
	#ifdef __WINDOWS

	int iPos =	szPath.find_last_of( "\\" );
	if( iPos > 0 )
	{
		szPath = szPath.substr( 0, iPos + 1 );
	}

	else
	{
		int iPos = szPath.find_last_of( "/" );
		if( iPos > 0 )
		{
			szPath = szPath.substr( 0, iPos + 1 );
		}

		else
			LOG( String( "App-name could not be removed for path " ) + szPath );
	}
	 

	#endif
}