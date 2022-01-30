#include "fancy_core_precompile.h"
#include "PathService.h"
#include "StaticString.h"

#include <ShlObj.h>
#include <codecvt>

#pragma comment(lib, "comsuppw")

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Path
  {
    static eastl::string ourRootFolder;
//---------------------------------------------------------------------------//
    /// The root folder is the folder the FANCY-folder is residing in, so that e.g. resources can be adressed with "Fancy/resources/..." for engine-resources or "resources/" for project-resources
    /// The relativeRootFolder should identify this folder relative to the .exe
    void InitRootFolder(const char* aRelativeRootFolder)
    {
      const eastl::string& appPath = Path::GetAppPath();

      ourRootFolder = appPath + "/" + aRelativeRootFolder;
      Path::RemoveFolderUpMarkers(ourRootFolder);
    }
//---------------------------------------------------------------------------//
    eastl::string GetAppName()
    {
      TCHAR buf[FILENAME_MAX] = {0};
      GetModuleFileName(NULL, buf, FILENAME_MAX);

      eastl::string str(buf);
      ConvertToSlash(str);

      size_t exePos = str.rfind(".exe");
      ASSERT(exePos != eastl::string::npos);

      size_t slashPos = str.rfind("/", exePos);

      return str.substr(slashPos + 1, exePos - slashPos - 1);
    }
  //---------------------------------------------------------------------------//
    eastl::string GetAppPath()
    {
      TCHAR outString[FILENAME_MAX];
      GetModuleFileName(NULL, outString, FILENAME_MAX);

      eastl::string pathOut(outString);
      ConvertToSlash(pathOut);
      return GetContainingFolder(pathOut);
    }
//---------------------------------------------------------------------------//
    eastl::string GetContainingFolder(const eastl::string& aPath)
    {
      size_t slashPos = (size_t) glm::min(aPath.rfind('/'), aPath.rfind('\\'));
      if (slashPos == eastl::string::npos)
        return aPath;

      return aPath.substr(0, slashPos);
    }
//---------------------------------------------------------------------------//
    const eastl::string& GetRootDirectory()
    {
      return ourRootFolder;
    }
//---------------------------------------------------------------------------//
    eastl::string GetWorkingDirectory()
    {
      TCHAR buf[MAX_PATH];
      if (!GetCurrentDirectory(MAX_PATH, buf))
        return "";

      eastl::string workingDir(buf);
      ConvertToSlash(workingDir);

      return workingDir;
    }
//---------------------------------------------------------------------------//
    eastl::string GetAbsolutePath(const eastl::string& aRelativePath)
    {
      if (IsPathAbsolute(aRelativePath))
        return aRelativePath;

      return GetRootDirectory() + aRelativePath;
    }
//---------------------------------------------------------------------------//
    eastl::string GetRelativePath(const eastl::string& anAbsolutePath)
    {
      if (!IsPathAbsolute(anAbsolutePath))
        return anAbsolutePath;

      const eastl::string& rootDir = GetRootDirectory();

      const size_t rootDirPath = anAbsolutePath.rfind(rootDir);

      if (rootDirPath == eastl::string::npos)
        return ""; // Path is absolute but not relative to our root directory

      return anAbsolutePath.substr(rootDirPath + rootDir.length());
    }
//---------------------------------------------------------------------------//
    bool FileExists(const char* aFilePath)
    {
      std::ifstream fileStream(aFilePath);
      return fileStream.good();
    }
//---------------------------------------------------------------------------//
    bool FileExists(const wchar_t* aFilePath)
    {
      std::ifstream fileStream(aFilePath);
      return fileStream.good();
    }
//---------------------------------------------------------------------------//
    bool IsPathAbsolute(const char* aPath)
    {
      return strlen(aPath) > 1 && aPath[1] == ':';
    }
//---------------------------------------------------------------------------//
    bool IsPathAbsolute(const eastl::string& aPath)
    {
      return aPath.size() > 1 && aPath[1] == ':';
    }
//---------------------------------------------------------------------------//
    eastl::string GetFileExtension(const eastl::string& szFileName)
    {
      size_t dotPos = szFileName.find_last_of(".");
      if (dotPos == eastl::string::npos)
        return "";

      return szFileName.substr(dotPos + 1, szFileName.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    eastl::string GetFilename(const eastl::string& aPath)
    {
      const size_t slashPos = glm::min(aPath.rfind('/'), aPath.rfind('\\'));
      const size_t dotPos = aPath.rfind(".");
      if (dotPos == eastl::string::npos)
      {
        if (slashPos == eastl::string::npos)
          return aPath;
      
        return aPath.substr(slashPos + 1);
      }

      if (slashPos == eastl::string::npos)
        return aPath.substr(0, aPath.size() - dotPos);
      
      return aPath.substr(slashPos + 1, aPath.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    eastl::string GetPathWithoutExtension(const eastl::string& aPath)
    {
      const size_t dotPos = aPath.rfind(".");
      if (dotPos == eastl::string::npos)
        return aPath;

      return aPath.substr(0, aPath.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    void ConvertToSlash(eastl::string& aPath)
    {
      std::replace(aPath.begin(), aPath.end(), '\\', '/');
    }
  //---------------------------------------------------------------------------//
    void ConvertToBackslash(eastl::string& aPath)
    {
      std::replace(aPath.begin(), aPath.end(), '/', '\\');
    }
 //---------------------------------------------------------------------------//
    void CreateDirectoryTreeForPath(const eastl::string& aPath)
    {
      eastl::string aDirectoryTree = GetContainingFolder(aPath) + "/";

      size_t posSlash = aDirectoryTree.find('/');
      if (posSlash != eastl::string::npos && IsPathAbsolute(aDirectoryTree))
      {
        // Skip the first slash
        posSlash = aDirectoryTree.find('/', posSlash + 1u);
      }

      while (posSlash != eastl::string::npos)
      {
        eastl::string currDirPath = aDirectoryTree.substr(0u, posSlash);
        CreateDirectory(currDirPath.c_str(), nullptr);

        posSlash = aDirectoryTree.find('/', posSlash + 1u);
      }
    }
    //---------------------------------------------------------------------------//
    void RemoveFolderUpMarkers(eastl::string& aPath)
    {
      ConvertToSlash(aPath);

      const eastl::string kSearchKey = "/../";
      const size_t kSearchKeyLen = kSearchKey.length();

      size_t posDots = aPath.find(kSearchKey);
      while (posDots != eastl::string::npos)
      {
        size_t posSlashBefore = aPath.rfind('/', posDots - 1u);
        if ((size_t)posSlashBefore == eastl::string::npos)
          posSlashBefore = -1;

        eastl::string firstPart = aPath.substr(0u, (size_t)(posSlashBefore + 1u));
        eastl::string secondPart = aPath.substr(posDots + kSearchKeyLen);
        aPath = firstPart + secondPart;

        posDots = aPath.find(kSearchKey);

        if (posSlashBefore == -1)
          break;
      }
    }
//---------------------------------------------------------------------------//
    uint64 GetFileWriteTime(const eastl::string& aFile)
    {
      uint64 lastWriteTimeStamp = 0u;

      HANDLE hFile = CreateFile(aFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

      if (hFile == INVALID_HANDLE_VALUE)
        return 0u;

      FILETIME lastWriteTime;
      const bool success = GetFileTime(hFile, nullptr, nullptr, &lastWriteTime) != 0;
      ASSERT(success, "File %s exists with a valid handle but failed to read its file time", aFile.c_str());

      CloseHandle(hFile);

      lastWriteTimeStamp = (static_cast<uint64>(lastWriteTime.dwHighDateTime) << 32) | lastWriteTime.dwLowDateTime;

      return lastWriteTimeStamp;
    }
  //---------------------------------------------------------------------------//
    eastl::string GetUserDataPath()
    {
      LPWSTR documentsFolder = NULL;
      if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsFolder)))
      {
        std::wstring str(documentsFolder);
        CoTaskMemFree(documentsFolder);
          
        eastl::string path(eastl::string::CtorConvert(), str.c_str());
        ConvertToSlash(path);

        return path + "/Fancy/" + GetAppName() + "/";
      }

      return "";
    }
  //---------------------------------------------------------------------------//
  }
}
