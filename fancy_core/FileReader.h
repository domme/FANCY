#pragma once

#include "FancyCorePrerequisites.h"
#include <list>

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
	  std::string ReadTextFile( const std::string& aPathAbs);
	  void ReadTextFileLines( const std::string& aPathAbs, std::vector<std::string>& someLinesOut);
    void ReadTextFileLines( const std::string& aPathAbs, std::list<std::string>& someLinesOut);
  };
//---------------------------------------------------------------------------//
}
