#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  // TODO: Refactor these functions to only accept const char* as parameters so they can be used with string-literals without creating a temp string

  namespace Path
  {
    void InitRootFolders();

    eastl::string GetAppName();
    eastl::string GetAppPath();
    eastl::string GetContainingFolder(const eastl::string& aFileName);
    
    const eastl::string& GetRootDirectory();
    eastl::string GetWorkingDirectory();
    eastl::string GetAbsolutePath(const eastl::string& aRelativePath);
    eastl::string GetRelativePath(const eastl::string& anAbsolutePath);
    eastl::string GetAbsoluteResourcePath(const eastl::string& aRelativeResourcePath, bool* aWasFound = nullptr);
    eastl::string GetRelativeResourcePath(const eastl::string& anAbsoluteResourcePath, bool* aWasFound = nullptr);
    bool FileExists(const char* aFilePath);
    bool FileExists(const wchar_t* aFilePath);
    bool IsPathAbsolute(const char* aPath);
    bool IsPathAbsolute(const eastl::string& aPath);
    eastl::string GetFileExtension(const eastl::string& aFileName);
    eastl::string GetFilename(const eastl::string& aPath);
    eastl::string GetPathWithoutExtension(const eastl::string& aPath);
    uint64 GetFileWriteTime(const eastl::string& aFile);
    eastl::string GetUserDataPath();

    /// The following two functions add quotes around path-segments that contain spaces so the path is accepted as input to command-lines
    eastl::string GetAsCmdParameter(const char* aPath);
    void PrepareForCmdParameter(eastl::string& aPath);

    void RemoveFolderUpMarkers(eastl::string& aPath);
    void ConvertToSlash(eastl::string& aPath);
    void ConvertToBackslash(eastl::string& aPath);
    void CreateDirectoryTreeForPath(const eastl::string& aPath);
  }
//---------------------------------------------------------------------------//
}