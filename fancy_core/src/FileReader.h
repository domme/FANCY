#ifndef INCLUDE_FILEREADER_H
#define INCLUDE_FILEREADER_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class FileReader
  {
	  public:			
	    //basic Text-Reading from file with no error-handling at all up till now
	    static std::string ReadTextFile( const std::string& szFileName );
	    static void ReadTextFileLines( const std::string& szFileName, std::vector<std::string>& rvLines, bool bInResources = true );
	    static void OpenFileStream( const std::string& szRelativeFileName, std::ifstream& rStream, bool bInResources = true );
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::IO 

#endif  // INCLUDE_FILEREADER_H