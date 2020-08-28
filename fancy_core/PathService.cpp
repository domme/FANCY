#include "fancy_core_precompile.h"
#include "PathService.h"
#include "StaticString.h"

#include <ShlObj.h>
#include <codecvt>

#include "eastl/vector.h"

#pragma comment(lib, "comsuppw")

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Path
  {
    static eastl::vector<eastl::string> ourResourceFolders;
    static eastl::string ourRootFolder;
//---------------------------------------------------------------------------//
    void InitRootFolders()
    {
      const eastl::string& appPath = Path::GetAppPath();
      ourRootFolder = StaticString<260>("%s/../../../", appPath.c_str());
      Path::RemoveFolderUpMarkers(ourRootFolder);
      
      const eastl::string appResourceFolder(StaticString<260>("%s%s/resources/", ourRootFolder.c_str(), Path::GetAppName().c_str()));
      const eastl::string coreResourceFolder(StaticString<260>("%sresources/", ourRootFolder.c_str()));

      // Folders are ordered in descending priority. 
      // Resources in earlier folders can "override" resources in later folders
      ourResourceFolders.clear();
      ourResourceFolders.push_back(appResourceFolder);
      ourResourceFolders.push_back(coreResourceFolder);
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

      return anAbsolutePath.substr(rootDirPath + 1);
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
    eastl::string GetAsCmdParameter(const char* aPath)
    {
      eastl::string path(aPath);
      PrepareForCmdParameter(path);
      return path;
    }
  //---------------------------------------------------------------------------//
    void PrepareForCmdParameter(eastl::string& aPath)
    {
      int i = 0;

      while (i < (int)aPath.size())
      {
        if (aPath[i] == ' ')
        {
          int iPrev = i;
          for (; iPrev > 0; --iPrev)
          {
            if (aPath[iPrev-1] == '/' || aPath[iPrev-1] == '\\')
              break;
          }

          aPath.insert(iPrev, "\"");
          ++i;

          int iNext = i;
          for (; iNext < (int) aPath.size(); ++iNext)
          {
            if (aPath[iNext] == '/' || aPath[iNext] == '\\')
              break;
          }

          aPath.insert(iNext, "\"");
        }

        ++i;
      }
    }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
    eastl::string GetAbsoluteResourcePath(const eastl::string& aRelativeResourcePath, bool* aWasFound /*=nullptr*/)
    {
      for (const eastl::string& resourceFolder : ourResourceFolders)
      {
        eastl::string resourcePath = resourceFolder + aRelativeResourcePath;
        if (Path::FileExists(resourcePath.c_str()))
        {
          if (aWasFound)
            *aWasFound = true;

          return resourcePath;
        }
      }

      // Fall back to the root dir if the resource hasn't been found in any of the registered resource folders
      const eastl::string& absPathInRootDir = Path::GetAbsolutePath(aRelativeResourcePath);
      const bool existsInRootDir = Path::FileExists(absPathInRootDir.c_str());

      if (aWasFound)
        *aWasFound = existsInRootDir;

      return existsInRootDir ? absPathInRootDir : "";
    }
//---------------------------------------------------------------------------//
    eastl::string GetRelativeResourcePath(const eastl::string& anAbsoluteResourcePath, bool* aWasFound /*=nullptr*/)
    {
      for (const eastl::string& resourceFolder : ourResourceFolders)
      {
        const size_t resourceFolderPos = anAbsoluteResourcePath.rfind(resourceFolder.c_str());
        if (resourceFolderPos != eastl::string::npos)
        {
          if (aWasFound)
            *aWasFound = true;

          return anAbsoluteResourcePath.substr(resourceFolderPos + resourceFolder.size());
        }
      }

      const eastl::string& relPathToRootDir = Path::GetRelativePath(anAbsoluteResourcePath);
      const bool isAbsoluteRootDirPath = !relPathToRootDir.empty();

      if (aWasFound)
        *aWasFound = isAbsoluteRootDirPath;

      return relPathToRootDir;
    }
//---------------------------------------------------------------------------//
  }
}
