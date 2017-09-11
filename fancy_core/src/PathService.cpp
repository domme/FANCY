#include "PathService.h"
#include <stdio.h>
#include <Windows.h>

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace PathUtil
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

      return str.substr(slashPos + 1, exePos - slashPos);
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

      return aPath.substr(0, aPath.size() - slashPos);
    }

    String GetWorkingDirectory()
    {
      TCHAR buf[MAX_PATH];
      if (GetCurrentDirectory(MAX_PATH, buf))
        return String(buf);

      return "";
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
      ASSERT(workingDirPos != String::npos);  // Path is not absolute but also not relative to our working directory

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
      for (uint32 i = 0; i < aPath.size(); ++i)
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
      String aDirectoryTree = GetContainingFolder(aPath);

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
      const uint32 kSearchKeyLen = kSearchKey.length();

      size_t posDots = aPath.find(kSearchKey);
      while (posDots != String::npos)
      {
        size_t posSlashBefore = aPath.rfind('/', posDots - 1u);
        if (posSlashBefore == String::npos)
        {
          break;
        }

        String firstPart = aPath.substr(0u, posSlashBefore + 1u);
        String secondPart = aPath.substr(posDots + kSearchKeyLen);
        aPath = firstPart + secondPart;

        posDots = aPath.find(kSearchKey);
      }
    }
    //---------------------------------------------------------------------------//
  }
  
  namespace ResourceUtil 
  {
    //---------------------------------------------------------------------------//
    static std::vector<String> ourResourceFolders;
    //---------------------------------------------------------------------------//
    void InitResourceFolders()
    {
      const String& appPath = PathUtil::GetAppPath();

      String appResourceFolder;
      appResourceFolder.Format("%../../%/%", appPath.c_str(), PathUtil::GetAppName().c_str());
      PathUtil::RemoveFolderUpMarkers(appResourceFolder);

      String coreResourceFolder;
      coreResourceFolder.Format("%../../resources", appPath.c_str());
      PathUtil::RemoveFolderUpMarkers(coreResourceFolder);

      // Folders are ordered in descending priority. 
      // Resources in earlier folders can "override" resources in later folders
      ourResourceFolders.clear();
      ourResourceFolders.push_back(appResourceFolder);
      ourResourceFolders.push_back(coreResourceFolder);
    }
  //---------------------------------------------------------------------------//
    bool FindResourcePath(const String& aResourceName, String& aResourcePathOut)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        String resourcePath = resourceFolder + aResourceName;
        if (PathUtil::FileExists(resourcePath.c_str()))
        {
          aResourcePathOut = resourcePath;
          return true;
        }
      }

      return false;
    }
  //---------------------------------------------------------------------------//
    bool GetResourceName(const String& aResourcePath, String& aResourceNameOut)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        const size_t resourceFolderPos = aResourcePath.rfind(resourceFolder.c_str());
        if (resourceFolderPos != String::npos)
        {
          aResourceNameOut = aResourcePath.substr(resourceFolderPos + 1);
          return true;
        }
      }
      
      return false;
    }
  //---------------------------------------------------------------------------//
  }
} }  // end of namespace Fancy::IO