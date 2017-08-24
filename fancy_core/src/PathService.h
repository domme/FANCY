#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace PathUtil
  {
    void InitResourceFolders();

    String GetAbsPath( const String& szRelPath, bool bInResources = true );
    void GetAbsPath( String& szRelPath, bool bInResources = true );
    String GetRelativePath(const String& _anAbsPath, bool _isInResources = true );
    String GetAppPath();
    String GetAppName();
    void GetResourceFolders(String& aCoreResourceFolderOut, String& anAppResourceFolderOut);
    bool IsPathAbs(const String& _szPath);
    String GetFileExtension( const String& szFileName );
    String GetContainingFolder( const String& szFileName );
    void RemoveFolderUpMarkers(String& _szPath);
    void UnifySlashes(String& _szPath);
    void RemoveFilenameFromPath(String& szPath);
    void CreateDirectoryTreeForPath(const String& _somePath);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO