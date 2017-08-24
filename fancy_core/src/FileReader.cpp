#include <fstream>
#include <sstream>
#include <vector>
#include <list>

#include "FileReader.h"
#include "PathService.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  std::string FileReader::ReadTextFile( const std::string& szFileName )
  {
    std::ifstream fileStream;
    OpenFileStream( szFileName, fileStream );
    std::stringstream stringStream;
    stringStream << fileStream.rdbuf();
    return stringStream.str();
  }
//---------------------------------------------------------------------------//
  void FileReader::ReadTextFileLines( const std::string& szFileName, std::vector<std::string>& rvLines, bool bInResources /* = true */ )
  {
    std::ifstream fileStream;
    OpenFileStream( szFileName, fileStream, bInResources );

    if( fileStream.good() )
    {
      while( !fileStream.eof() )
      {
        rvLines.push_back(std::string());
        std::getline( fileStream, rvLines[rvLines.size() - 1]);
      }		
    }
  }
//---------------------------------------------------------------------------//
  void FileReader::ReadTextFileLines( const std::string& szFileName, std::list<std::string>& rvLines, bool bInResources /* = true */ )
  {
    std::ifstream fileStream;
    OpenFileStream( szFileName, fileStream, bInResources );

    if( fileStream.good() )
    {
      while( !fileStream.eof() )
      {
        std::string line;
        std::getline( fileStream, line);
        rvLines.push_back(line);
      }
    }
  }
//---------------------------------------------------------------------------//
  void FileReader::OpenFileStream(const std::string& szRelativeFileName, std::ifstream& rStream, bool bInResources /* = true */ )
  {
    std::string path = PathService::GetAbsPath( szRelativeFileName, bInResources );
    rStream.open( path.c_str() );
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::IO

