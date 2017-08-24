#include "PathService.h"
#include <stdio.h>
#include <Windows.h>

#include "FixedArray.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace PathUtil
  {
    static String ourCoreResourceFolder;
    static String ourAppResourceFolder;

    static const char* kResourceFolderName = "resources";

    void InitResourceFolders()
    {
      const String& appPath = GetExePath();
      ourCoreResourceFolder.Format("%../../%", appPath.c_str(), kResourceFolderName);

      
    }

    String GetAppName()
    {
      String path = "";

      TCHAR buf[FILENAME_MAX];
      int bytes = GetModuleFileName(NULL, buf, FILENAME_MAX);
      path = buf;

      
      UnifySlashes(path);
      int iPosFrom = path.find_last_of("/");
      int iPosTo = path
      return path.substr(iPos + 1, path.size() - iPos);
    }

    String GetExePath()
    {
      String pathOut = "";

      TCHAR outString[FILENAME_MAX];
      int bytes = GetModuleFileName(NULL, outString, FILENAME_MAX);

      pathOut = outString;
      RemoveFilenameFromPath(pathOut);

      return pathOut;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetContainingFolder(const String& szFileName)
    {
      String szFolderPath = szFileName;
      RemoveFilenameFromPath(szFolderPath);
      return szFolderPath;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetAbsPath(const String& szRelPath, bool bInResources /* = true */)
    {
      if (bInResources)
        return GetResourceFolders() + szRelPath;

      else
        return GetExePath() + szRelPath;
    }
    //---------------------------------------------------------------------------//
    void PathService::GetAbsPath(String& szRelPath, bool bInResources /* = true */)
    {
      if (bInResources)
        szRelPath = GetResourceFolders() + szRelPath;

      else
        szRelPath = GetExePath() + szRelPath;
    }
    //---------------------------------------------------------------------------//
    String PathService::GetRelativePath(const String& _anAbsPath, bool _isInResources)
    {
      const String theAbsPart = _isInResources ? GetResourceFolders() : GetExePath();

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

      RemoveFolderUpMarkers(szPath);

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
    void PathService::RemoveFilenameFromPath(String& szPath)
    {
      UnifySlashes(szPath);
      size_t posDot = szPath.find_last_of('.');
      if (posDot == String::npos)
      {
        return;
      }

      size_t posLastSlash = szPath.find_last_of("/");
      if (posLastSlash != String::npos && posDot > posLastSlash)
      {
        szPath = szPath.substr(0u, posLastSlash + 1u);
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
    void PathService::RemoveFolderUpMarkers(String& _szPath)
    {
      UnifySlashes(_szPath);

      const String kSearchKey = "/../";
      const uint32 kSearchKeyLen = kSearchKey.length();

      size_t posDots = _szPath.find(kSearchKey);
      while (posDots != String::npos)
      {
        size_t posSlashBefore = _szPath.rfind('/', posDots - 1u);
        if (posSlashBefore == String::npos)
        {
          break;
        }

        String firstPart = _szPath.substr(0u, posSlashBefore + 1u);
        String secondPart = _szPath.substr(posDots + kSearchKeyLen);
        _szPath = firstPart + secondPart;

        posDots = _szPath.find(kSearchKey);
      }
    }
    //---------------------------------------------------------------------------//
  }
} }  // end of namespace Fancy::IO