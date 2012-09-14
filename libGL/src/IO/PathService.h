#ifndef PATHSERVICE_H
#define PATHSERVICE_H

#include "../includes.h"

class DLLEXPORT PathService
{
public:
	static String convertToAbsPath( const String& szRelPath, bool bInResources = true );
	static void convertToAbsPath( String& szRelPath, bool bInResources = true );
	static String getExePath();
	static String getResourcesPath();
	static String getFileType( const String& szFileName );
	static String GetContainingFolder( const String& szFileName );
	static void SetResourceLocation( const String& szResource );
	

private:
	static void removeAppName( String& szPath );
	static std::string m_szRelativeResourcePath;

	

	
};

#endif