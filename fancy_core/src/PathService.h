#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace PathUtil
  {
    String GetAppName();
    String GetAppPath();
    String GetContainingFolder(const String& szFileName);
    
    void GetResourceFolders(String& aCoreResourceFolderOut, String& anAppResourceFolderOut);
    bool IsPathAbs(const String& _szPath);
    String GetFileExtension( const String& szFileName );
    
    void RemoveNavElementsFromPath(String& _szPath);
    void UnifySlashes(String& _szPath);
    void CreateDirectoryTreeForPath(const String& _somePath);
  }
//---------------------------------------------------------------------------//


  namespace ResourceUtil
  {
    void InitResourceFolders();

    String FindResourcePath(const String& aResourceName);
    String GetResourceName(const String& aResourcePath);
  }



} }  // end of namespace Fancy::IO