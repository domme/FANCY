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
    static bool isAbsolutePath(const String& _szPath);
    static String getFileExtension( const String& szFileName );
    static String GetContainingFolder( const String& szFileName );
    static void SetResourceLocation( const String& szResource );
    static void removeFolderUpMarkers(String& _szPath);
    static void unifySlashes(String& _szPath);
    static void removeFilenameFromPath( String& szPath );

  private:
    static std::string m_szRelativeResourcePath;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_PATHSERVICE_H