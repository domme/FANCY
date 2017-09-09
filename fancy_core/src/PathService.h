#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace PathUtil
  {
    String GetAppName();
    String GetAppPath();
    String GetContainingFolder(const String& aFileName);
    
    bool IsPathAbsolute(const String& aPath);
    String GetFileExtension(const String& aFileName);
    String GetFilenameWithoutExtension(const String& aPath);
    
    void RemoveNavElementsFromPath(String& aPath);
    void UnifySlashes(String& aPath);
    void CreateDirectoryTreeForPath(const String& aPath);
  }
//---------------------------------------------------------------------------//
  namespace ResourceUtil
  {
    void InitResourceFolders();

    bool FindResourcePath(const String& aResourceName, String& aResourcePathOut);
    bool GetResourceName(const String& aResourcePath, String & aResourceNameOut);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO