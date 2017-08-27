#include "PathService.h"
#include <stdio.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "FixedArray.h"
#include <Shlwapi.h>
#include <Shlwapi.h>

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
    String GetAbsPath(const String& szRelPath, bool bInResources /* = true */)
    {
      if (bInResources)
      {
        if (Exist)
      }
        return GetResourceFolders() + szRelPath;

      else
        return GetAppPath() + szRelPath;
    }
    //---------------------------------------------------------------------------//
    void PathService::GetAbsPath(String& szRelPath, bool bInResources /* = true */)
    {
      if (bInResources)
        szRelPath = GetResourceFolders() + szRelPath;

      else
        szRelPath = GetAppPath() + szRelPath;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetRelativePath(const String& _anAbsPath, bool _isInResources)
    {
      const String theAbsPart = _isInResources ? GetResourceFolders() : GetAppPath();

      std::size_t thePosOfAbsPart = _anAbsPath.find(theAbsPart);

      if (thePosOfAbsPart != String::npos)
      {
        return _anAbsPath.substr(thePosOfAbsPart + theAbsPart.length());
      }

      return _anAbsPath;
    }
    //---------------------------------------------------------------------------//
    bool PathService::IsPathAbs(const String& _szPath)
    {
      return _szPath.size() > 2u && _szPath[1] == ':';
    }
    //---------------------------------------------------------------------------//
    void PathService::SetResourceLocation(const String& szResource)
    {
      m_szRelativeResourcePath = szResource;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetResourceFolders()
    {
      static String szPath = "";

      if (szPath != "")
      {
        return szPath;
      }

      szPath = GetExePath();

      if (m_szRelativeResourcePath.empty())
      {
        szPath += "/Resources/";
      }
      else
      {
        szPath += m_szRelativeResourcePath;
      }

      RemoveNavElementsFromPath(szPath);

      return szPath;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetFileExtension(const String& szFileName)
    {
      int iPos = szFileName.find_last_of(".");
      return szFileName.substr(iPos + 1, szFileName.size() - iPos);
    }
    //---------------------------------------------------------------------------//
    void PathService::UnifySlashes(String& _szPath)
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
    void PathService::CreateDirectoryTreeForPath(const String& _somePath)
    {
      String aDirectoryTree = GetContainingFolder(_somePath);

      size_t posSlash = aDirectoryTree.find('/');
      if (posSlash != String::npos && IsPathAbs(aDirectoryTree))
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
    String FindResourcePath(const String& aResourceName)
    {
      for (const String& resourceFolder : ourResourceFolders)
      {
        String resourcePath = resourceFolder + aResourceName;
        if (PathFileExists(resourcePath.c_str()) == 1)
          return resourcePath;
      }

      return "";
    }
  //---------------------------------------------------------------------------//
    String GetResourceName(const String& aResourcePath)
    {
      TCHAR commonPrefixPathBuf[FILENAME_MAX];
      for (const String& resourceFolder : ourResourceFolders)
      {
        if (PathCommonPrefix(resourceFolder.c_str(), aResourcePath.c_str(), commonPrefixPathBuf) > 0)
        {
          if (strcmp(resourceFolder.c_str(), commonPrefixPathBuf) == 0)
            return PathRelativePathTo()
        }
      }
    }
  //---------------------------------------------------------------------------//

  }

} }  // end of namespace Fancy::IO