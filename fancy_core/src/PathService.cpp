#include "PathService.h"
#include <stdio.h>
#include <Windows.h>

#include "FixedArray.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  std::string PathService::m_szRelativeResourcePath;

  String PathService::getExePath()
  {
    String pathOut = "";

    TCHAR outString[FILENAME_MAX];
    int bytes = GetModuleFileName( NULL, outString, FILENAME_MAX );

    pathOut = outString;
    removeFilenameFromPath( pathOut );

    return pathOut;
  }
//---------------------------------------------------------------------------//
  String PathService::GetContainingFolder( const String& szFileName )
  {
    String szFolderPath = szFileName;
    removeFilenameFromPath( szFolderPath );
    return szFolderPath;
  }
//---------------------------------------------------------------------------//
  String PathService::convertToAbsPath( const String& szRelPath, bool bInResources /* = true */ )
  {
    if( bInResources )
      return getResourcesPath() + szRelPath;

    else
      return getExePath() + szRelPath;
  }
//---------------------------------------------------------------------------//
  void PathService::convertToAbsPath( String& szRelPath, bool bInResources /* = true */ )
  {
    if( bInResources )
      szRelPath = getResourcesPath() + szRelPath;

    else
      szRelPath = getExePath() + szRelPath;
  }
//---------------------------------------------------------------------------//
  String PathService::toRelPath(const String& _anAbsPath, bool _isInResources)
  {
    const String theAbsPart = _isInResources ? getResourcesPath() : getExePath();

    std::size_t thePosOfAbsPart = _anAbsPath.find(theAbsPart);

    if (thePosOfAbsPart != String::npos)
    {
      return _anAbsPath.substr(thePosOfAbsPart + theAbsPart.length());
    }

    return _anAbsPath;
  }
//---------------------------------------------------------------------------//
  bool PathService::isAbsolutePath(const String& _szPath)
  {
    return _szPath.size() > 2u && _szPath[1] == ':';
  }
//---------------------------------------------------------------------------//
  void PathService::SetResourceLocation( const String& szResource )
  {
    m_szRelativeResourcePath = szResource;
  }
//---------------------------------------------------------------------------//
  String PathService::getResourcesPath()
  {
    static String szPath = "";

    if( szPath != "" )
    {
      return szPath;
    }

    szPath = getExePath();

    if( m_szRelativeResourcePath.empty() )
    {
      szPath += "/Resources/";
    }
    else
    {
      szPath += m_szRelativeResourcePath;
    }

    removeFolderUpMarkers(szPath);

    return szPath;
  }
//---------------------------------------------------------------------------//
  String PathService::getFileExtension( const String& szFileName )
  {
    int iPos = szFileName.find_last_of( "." );
    return szFileName.substr( iPos + 1, szFileName.size() - iPos );
  }
//---------------------------------------------------------------------------//
  void PathService::unifySlashes(String& _szPath)
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
  void PathService::removeFilenameFromPath( String& szPath )
  {
    unifySlashes(szPath);
    size_t posDot = szPath.find_last_of('.');
    if (posDot == String::npos)
    {
      return;
    }

    size_t posLastSlash =	szPath.find_last_of( "/" );
    if( posLastSlash != String::npos && posDot > posLastSlash )
    {
      szPath = szPath.substr( 0u, posLastSlash + 1u );
    }
  }
//---------------------------------------------------------------------------//
  void PathService::createDirectoryTreeForPath(const String& _somePath)
  {
    String aDirectoryTree = GetContainingFolder(_somePath);

    size_t posSlash = aDirectoryTree.find('/');
    if (posSlash != String::npos && isAbsolutePath(aDirectoryTree))
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
  void PathService::removeFolderUpMarkers( String& _szPath )
  {
    unifySlashes(_szPath);

    const String kSearchKey = "/../";
    const uint32 kSearchKeyLen = kSearchKey.length();

    size_t posDots = _szPath.find(kSearchKey);
    while(posDots != String::npos)
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
} }  // end of namespace Fancy::IO