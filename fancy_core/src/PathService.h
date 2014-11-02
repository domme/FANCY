#ifndef INCLUDE_PATHSERVICE_H
#define INCLUDE_PATHSERVICE_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class PathService
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
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_PATHSERVICE_H