#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  // TODO: Refactor these functions to only accept const char* as parameters so they can be used with string-literals without creating a temp string

  namespace Path
  {
    void InitRootFolders();

    String GetAppName();
    String GetAppPath();
    String GetContainingFolder(const String& aFileName);
    
    const String& GetRootDirectory();
    String GetWorkingDirectory();
    String GetAbsolutePath(const String& aRelativePath);
    String GetRelativePath(const String& anAbsolutePath);
    String GetAbsoluteResourcePath(const String& aRelativeResourcePath, bool* aWasFound = nullptr);
    String GetRelativeResourcePath(const String& anAbsoluteResourcePath, bool* aWasFound = nullptr);
    bool FileExists(const char* aFilePath);
    bool FileExists(const wchar_t* aFilePath);
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
    void ConvertToSlash(String& aPath);
    void ConvertToBackslash(String& aPath);
    void CreateDirectoryTreeForPath(const String& aPath);
  }
//---------------------------------------------------------------------------//
}