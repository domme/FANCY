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
    static std::vector<String> ourResourceFolders;
    static String ourRootFolder;
//---------------------------------------------------------------------------//
    void InitRootFolders()
    {
      const String& appPath = Path::GetAppPath();
      ourRootFolder = StaticString<260>("%s/../../../", appPath.c_str());
      Path::RemoveFolderUpMarkers(ourRootFolder);
      
      const String appResourceFolder(StaticString<260>("%s%s/resources/", ourRootFolder.c_str(), Path::GetAppName().c_str()));
      const String coreResourceFolder(StaticString<260>("%sresources/", ourRootFolder.c_str()));

      // Folders are ordered in descending priority. 
      // Resources in earlier folders can "override" resources in later folders
      ourResourceFolders.clear();
      ourResourceFolders.push_back(appResourceFolder);
      ourResourceFolders.push_back(coreResourceFolder);
    }
//---------------------------------------------------------------------------//
    String GetAppName()
    {
      TCHAR buf[FILENAME_MAX] = {0};
      GetModuleFileName(NULL, buf, FILENAME_MAX);

      String str(buf);
      ConvertToSlash(str);

      size_t exePos = str.rfind(".exe");
      ASSERT(exePos != String::npos);

      size_t slashPos = str.rfind("/", exePos);

      return str.substr(slashPos + 1, exePos - slashPos - 1);
    }
  //---------------------------------------------------------------------------//
    String GetAppPath()
    {
      TCHAR outString[FILENAME_MAX];
      GetModuleFileName(NULL, outString, FILENAME_MAX);

      String pathOut(outString);
      ConvertToSlash(pathOut);
      return GetContainingFolder(pathOut);
    }
//---------------------------------------------------------------------------//
    String GetContainingFolder(const String& aPath)
    {
      size_t slashPos = (size_t) glm::min(aPath.rfind('/'), aPath.rfind('\\'));
      if (slashPos == String::npos)
        return aPath;

      return aPath.substr(0, slashPos);
    }
//---------------------------------------------------------------------------//
    const String& GetRootDirectory()
    {
      return ourRootFolder;
    }
//---------------------------------------------------------------------------//
    String GetWorkingDirectory()
    {
      TCHAR buf[MAX_PATH];
      if (!GetCurrentDirectory(MAX_PATH, buf))
        return "";

      String workingDir(buf);
      ConvertToSlash(workingDir);

      return workingDir;
    }
//---------------------------------------------------------------------------//
    String GetAbsolutePath(const String& aRelativePath)
    {
      if (IsPathAbsolute(aRelativePath))
        return aRelativePath;

      return GetRootDirectory() + aRelativePath;
    }
//---------------------------------------------------------------------------//
    String GetRelativePath(const String& anAbsolutePath)
    {
      if (!IsPathAbsolute(anAbsolutePath))
        return anAbsolutePath;

      const String& rootDir = GetRootDirectory();

      const size_t rootDirPath = anAbsolutePath.rfind(rootDir);

      if (rootDirPath == String::npos)
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
    bool IsPathAbsolute(const String& aPath)
    {
      return aPath.size() > 1 && aPath[1] == ':';
    }
//---------------------------------------------------------------------------//
    String GetFileExtension(const String& szFileName)
    {
      size_t dotPos = szFileName.find_last_of(".");
      if (dotPos == String::npos)
        return "";

      return szFileName.substr(dotPos + 1, szFileName.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    String GetFilename(const String& aPath)
    {
      const size_t slashPos = glm::min(aPath.rfind('/'), aPath.rfind('\\'));
      const size_t dotPos = aPath.rfind(".");
      if (dotPos == String::npos)
      {
        if (slashPos == String::npos)
          return aPath;
      
        return aPath.substr(slashPos + 1);
      }

      if (slashPos == String::npos)
        return aPath.substr(0, aPath.size() - dotPos);
      
      return aPath.substr(slashPos + 1, aPath.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    String GetPathWithoutExtension(const String& aPath)
    {
      const size_t dotPos = aPath.rfind(".");
      if (dotPos == String::npos)
        return aPath;

      return aPath.substr(0, aPath.size() - dotPos);
    }
//---------------------------------------------------------------------------//
    void ConvertToSlash(String& aPath)
    {
      std::replace(aPath.begin(), aPath.end(), '\\', '/');
    }
  //---------------------------------------------------------------------------//
    void ConvertToBackslash(String& aPath)
    {
      std::replace(aPath.begin(), aPath.end(), '/', '\\');
    }
 //---------------------------------------------------------------------------//
    void CreateDirectoryTreeForPath(const String& aPath)
    {
      String aDirectoryTree = GetContainingFolder(aPath) + "/";

      size_t posSlash = aDirectoryTree.find('/');
      if (posSlash != String::npos && IsPathAbsolute(aDirectoryTree))
      {
        // Skip the first slash
        posSlash = aDirectoryTree.find('/', posSlash + 1u);
      }

      while (posSlash != String::npos)
      {
        String currDirPath = aDirectoryTree.substr(0u, posSlash);
        CreateDirectory(currDirPath.c_str(), nullptr);

        posSlash = aDirectoryTree.find('/', posSlash + 1u);
      }
    }
    //---------------------------------------------------------------------------//
    void RemoveFolderUpMarkers(String& aPath)
    {
      ConvertToSlash(aPath);

      const String kSearchKey = "/../";
      const size_t kSearchKeyLen = kSearchKey.length();

      size_t posDots = aPath.find(kSearchKey);
      while (posDots != String::npos)
      {
        size_t posSlashBefore = aPath.rfind('/', posDots - 1u);
        if ((size_t)posSlashBefore == String::npos)
          posSlashBefore = -1;

        String firstPart = aPath.substr(0u, (size_t)(posSlashBefore + 1u));
        String secondPart = aPath.substr(posDots + kSearchKeyLen);
        aPath = firstPart + secondPart;

        posDots = aPath.find(kSearchKey);

        if (posSlashBefore == -1)
          break;
      }
    }
//---------------------------------------------------------------------------//
    uint64 GetFileWriteTime(const String& aFile)
    {
      uint64 lastWriteTimeStamp = 0u;

#if __WINDOWS
      HANDLE hFile = CreateFile(aFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

      if (hFile == INVALID_HANDLE_VALUE)
        return 0u;

      FILETIME lastWriteTime;
      const bool success = GetFileTime(hFile, nullptr, nullptr, &lastWriteTime) != 0;
      ASSERT(success, "File %s exists with a valid handle but failed to read its file time", aFile.c_str());

      CloseHandle(hFile);

      lastWriteTimeStamp = (static_cast<uint64>(lastWriteTime.dwHighDateTime) << 32) | lastWriteTime.dwLowDateTime;
#endif

      return lastWriteTimeStamp;
    }
  //---------------------------------------------------------------------------//
    String GetUserDataPath()
    {
      LPWSTR documentsFolder = NULL;
      if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsFolder)))
      {
        std::wstring str(documentsFolder);
        CoTaskMemFree(documentsFolder);
          
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        String path = converter.to_bytes(str);
        ConvertToSlash(path);

        return path + "/Fancy/" + GetAppName() + "/";
      }

      return "";
    }
  //---------------------------------------------------------------------------//
    String GetAsCmdParameter(const char* aPath)
    {
      String path(aPath);
      PrepareForCmdParameter(path);
      return path;
    }
  //---------------------------------------------------------------------------//
    void PrepareForCmdParameter(String& aPath)
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
    String GetAbsoluteResourcePath(const String& aRelativeResourcePath, bool* aWasFound /*=nullptr*/)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        String resourcePath = resourceFolder + aRelativeResourcePath;
        if (Path::FileExists(resourcePath.c_str()))
        {
          if (aWasFound)
            *aWasFound = true;

          return resourcePath;
        }
      }

      // Fall back to the root dir if the resource hasn't been found in any of the registered resource folders
      const String& absPathInRootDir = Path::GetAbsolutePath(aRelativeResourcePath);
      const bool existsInRootDir = Path::FileExists(absPathInRootDir.c_str());

      if (aWasFound)
        *aWasFound = existsInRootDir;

      return existsInRootDir ? absPathInRootDir : "";
    }
//---------------------------------------------------------------------------//
    String GetRelativeResourcePath(const String& anAbsoluteResourcePath, bool* aWasFound /*=nullptr*/)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        const size_t resourceFolderPos = anAbsoluteResourcePath.rfind(resourceFolder.c_str());
        if (resourceFolderPos != String::npos)
        {
          if (aWasFound)
            *aWasFound = true;

          return anAbsoluteResourcePath.substr(resourceFolderPos + resourceFolder.size());
        }
      }

      const String& relPathToRootDir = Path::GetRelativePath(anAbsoluteResourcePath);
      const bool isAbsoluteRootDirPath = !relPathToRootDir.empty();

      if (aWasFound)
        *aWasFound = isAbsoluteRootDirPath;

      return relPathToRootDir;
    }
//---------------------------------------------------------------------------//
  }
}
