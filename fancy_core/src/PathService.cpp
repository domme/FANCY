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

      TCHAR* fileName = PathFindFileName(buf);  // "myApp.exe"
      *PathFindExtension(fileName) = 0;  // "myApp"

      return String(fileName);
    }
  //---------------------------------------------------------------------------//
    String GetAppPath()
    {
      TCHAR outString[FILENAME_MAX];
      GetModuleFileName(NULL, outString, FILENAME_MAX);

      String pathOut(outString);
      return GetContainingFolder(pathOut);
    }
//---------------------------------------------------------------------------//
    String GetContainingFolder(const String& szFileName)
    {
      ASSERT(szFileName.size() <= FILENAME_MAX);

      TCHAR buf[FILENAME_MAX] = { 0 };
      memcpy(buf, szFileName.c_str(), szFileName.size() * sizeof(char));

      PathRemoveFileSpec(buf);
      return String(buf);
    }
//---------------------------------------------------------------------------//
    bool IsPathAbsolute(const String& aPath)
    {
      return !PathIsRelative(aPath.c_str());
    }
//---------------------------------------------------------------------------//
    String GetFileExtension(const String& szFileName)
    {
      int iPos = szFileName.find_last_of(".");
      return szFileName.substr(iPos + 1, szFileName.size() - iPos);
    }
//---------------------------------------------------------------------------//
    String GetFilenameWithoutExtension(const String& aPath)
    {
      int iPos = szFileName.find_last_of(".");
      return szFileName.substr(iPos + 1, szFileName.size() - iPos);
    }
//---------------------------------------------------------------------------//
    void UnifySlashes(String& _szPath)
    {
      for (uint32 i = 0; i < _szPath.size(); ++i)
      {
        if (_szPath[i] == '\\')
        {
          _szPath[i] = '/';
        }
      }
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
    void RemoveNavElementsFromPath(String& _szPath)
    {
      ASSERT(_szPath.size() <= FILENAME_MAX);
      
      TCHAR out[FILENAME_MAX];
      PathCanonicalize(out, _szPath.c_str());
      _szPath = out;
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
      PathUtil::RemoveNavElementsFromPath(appResourceFolder);

      String coreResourceFolder;
      coreResourceFolder.Format("%../../resources", appPath.c_str());
      PathUtil::RemoveNavElementsFromPath(coreResourceFolder);

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
        if (PathFileExists(resourcePath.c_str()) == 1)
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
      TCHAR commonPrefixPathBuf[FILENAME_MAX];
      for (const String& resourceFolder : ourResourceFolders)
      {
        const int prefixNumChars = PathCommonPrefix(resourceFolder.c_str(), aResourcePath.c_str(), commonPrefixPathBuf);
        if (prefixNumChars > 0)
        {
          aResourceNameOut = aResourcePath.substr(prefixNumChars);
          return true;
        }
      }

      return false;
    }
  //---------------------------------------------------------------------------//
  }
} }  // end of namespace Fancy::IO