#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  // TODO: Refactor these functions to only accept const char* as parameters so they can be used with string-literals without creating a temp string

  namespace Path
  {
    String GetAppName();
    String GetAppPath();
    String GetContainingFolder(const String& aFileName);
    
    String GetWorkingDirectory();
    String GetAbsolutePath(const String& aWorkingDirPath);
    String GetRelativePath(const String& anAbsolutePath);
    bool FileExists(const char* aFilePath);
    bool IsPathAbsolute(const char* aPath);
    bool IsPathAbsolute(const String& aPath);
    String GetFileExtension(const String& aFileName);
    String GetFilename(const String& aPath);
    String GetPathWithoutExtension(const String& aPath);
    uint64 GetFileWriteTime(const String& aFile);
    String GetUserDataPath();

    /// The following two functions add quotes around path-segments that contain spaces so the path is accepted as input to command-lines
    String GetAsCmdParameter(const char* aPath);
    void PrepareForCmdParameter(String& aPath);

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