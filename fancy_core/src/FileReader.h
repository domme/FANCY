#ifndef INCLUDE_FILEREADER_H
#define INCLUDE_FILEREADER_H

#include "FancyCorePrerequisites.h"

#include <list>

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
	  std::string ReadTextFile( const std::string& aPathAbs);
	  void ReadTextFileLines( const std::string& aPathAbs, std::vector<std::string>& someLinesOut);
    void ReadTextFileLines( const std::string& aPathAbs, std::list<std::string>& someLinesOut);
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::IO 

#endif  // INCLUDE_FILEREADER_H