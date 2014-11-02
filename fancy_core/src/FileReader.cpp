#include <fstream>
#include <sstream>
#include <vector>

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
			  std::string newLine;
			  std::getline( fileStream, newLine );
			  rvLines.push_back( newLine );		
		  }		
	  }
  }
//---------------------------------------------------------------------------//
  void FileReader::OpenFileStream(const std::string& szRelativeFileName, std::ifstream& rStream, bool bInResources /* = true */ )
  {
	  std::string path = PathService::convertToAbsPath( szRelativeFileName, bInResources );
	  rStream.open( path.c_str() );
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::IO

