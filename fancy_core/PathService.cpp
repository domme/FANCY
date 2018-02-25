#include "PathService.h"
#include <stdio.h>
#include <Windows.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Path
  {
  //---------------------------------------------------------------------------//
    String GetAppName()
    {
      TCHAR buf[FILENAME_MAX] = {0};
      GetModuleFileName(NULL, buf, FILENAME_MAX);

      String str(buf);
      UnifySlashes(str);

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
      UnifySlashes(pathOut);
      return GetContainingFolder(pathOut);
    }
//---------------------------------------------------------------------------//
    String GetContainingFolder(const String& aPath)
    {
      size_t slashPos = glm::min(aPath.rfind('/'), aPath.rfind('\\'));
      if (slashPos == String::npos)
        return aPath;

      return aPath.substr(0, slashPos);
    }
//---------------------------------------------------------------------------//
    String GetWorkingDirectory()
    {
      TCHAR buf[MAX_PATH];
      if (!GetCurrentDirectory(MAX_PATH, buf))
        return "";

      String workingDir(buf);
      UnifySlashes(workingDir);

      return workingDir;
    }
//---------------------------------------------------------------------------//
    String GetAbsolutePath(const String& aWorkingDirPath)
    {
      if (IsPathAbsolute(aWorkingDirPath))
        return aWorkingDirPath;

      return GetWorkingDirectory() + "/" + aWorkingDirPath;
    }
//---------------------------------------------------------------------------//
    String GetRelativePath(const String& anAbsolutePath)
    {
      if (!IsPathAbsolute(anAbsolutePath))
        return anAbsolutePath;

      const String& workingDir = GetWorkingDirectory();

      const size_t workingDirPos = anAbsolutePath.rfind(workingDir);

      if (workingDirPos == String::npos)
        return ""; // Path is absolute but not relative to our working directory

      return anAbsolutePath.substr(workingDirPos + 1);
    }
//---------------------------------------------------------------------------//
    bool FileExists(const String& aFilePath)
    {
      if (FILE *file = fopen(aFilePath.c_str(), "r")) 
      {
        fclose(file);
        return true;
      }

      return false;
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
    void UnifySlashes(String& aPath)
    {
      for (uint i = 0; i < aPath.size(); ++i)
        if (aPath[i] == '\\')
          aPath[i] = '/';
    }
  //---------------------------------------------------------------------------//
    bool HasUnifiedSlashes(const String& aPath)
    {
      return aPath.find('\\') == String::npos;
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
      UnifySlashes(aPath);

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
  }
  
  namespace Resources 
  {
    //---------------------------------------------------------------------------//
    static std::vector<String> ourResourceFolders;
    //---------------------------------------------------------------------------//
    void InitResourceFolders()
    {
      const String& appPath = Path::GetAppPath();

      String appResourceFolder;
      appResourceFolder.Format("%/../../../%/", appPath.c_str(), Path::GetAppName().c_str());
      Path::RemoveFolderUpMarkers(appResourceFolder);

      String coreResourceFolder;
      coreResourceFolder.Format("%/../../../resources/", appPath.c_str());
      Path::RemoveFolderUpMarkers(coreResourceFolder);

      // Folders are ordered in descending priority. 
      // Resources in earlier folders can "override" resources in later folders
      ourResourceFolders.clear();
      ourResourceFolders.push_back(appResourceFolder);
      ourResourceFolders.push_back(coreResourceFolder);
    }
  //---------------------------------------------------------------------------//
    String FindPath(const String& aResourceName, bool* aWasFound /*=nullptr*/)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        String resourcePath = resourceFolder + aResourceName;
        if (Path::FileExists(resourcePath.c_str()))
        {
          if (aWasFound)
            *aWasFound = true;

          return resourcePath;
        }
      }

      // Fall back to the working dir if the resource hasn't been found in any of the registered resource folders
      const String& absPathInWorkDir = Path::GetAbsolutePath(aResourceName);
      const bool existsInWorkDir = Path::FileExists(absPathInWorkDir);

      if (aWasFound)
        *aWasFound = existsInWorkDir;

      return existsInWorkDir ? absPathInWorkDir : "";
    }
  //---------------------------------------------------------------------------//
    String FindName(const String& anAbsoluteResourcePath, bool* aWasFound /*=nullptr*/)
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

      const String& relPathWorkDir = Path::GetRelativePath(anAbsoluteResourcePath);
      const bool isAbsoluteWorkDirPath = relPathWorkDir.size() > 0;
      
      if (aWasFound)
        *aWasFound = isAbsoluteWorkDirPath;
      
      return relPathWorkDir;
    }
//---------------------------------------------------------------------------//
  }
}