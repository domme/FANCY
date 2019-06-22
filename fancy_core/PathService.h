#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Path
  {
    String GetAppName();
    String GetAppPath();
    String GetContainingFolder(const String& aFileName);
    
    String GetWorkingDirectory();
    String GetAbsolutePath(const String& aWorkingDirPath);
    String GetRelativePath(const String& anAbsolutePath);
    bool FileExists(const String& aFilePath);
    bool IsPathAbsolute(const String& aPath);
    String GetFileExtension(const String& aFileName);
    String GetFilename(const String& aPath);
    String GetPathWithoutExtension(const String& aPath);
    uint64 GetFileWriteTime(const String& aFile);
    String GetUserDataPath();
        
    void RemoveFolderUpMarkers(String& aPath);
    void UnifySlashes(String& aPath);
    bool HasUnifiedSlashes(const String& aPath);
    void CreateDirectoryTreeForPath(const String& aPath);
  }
//---------------------------------------------------------------------------//
  namespace Resources
  {
    void InitResourceFolders();

    String FindPath(const String& aRelativeResourcePath, bool* aWasFound = nullptr);
    String FindName(const String& anAbsoluteResourcePath, bool* aWasFound = nullptr);
  }
//---------------------------------------------------------------------------//
}