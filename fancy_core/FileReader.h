#pragma once

#include <list>
#include <vector>
#include <string>

#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
	  std::string ReadTextFile( const std::string& aPathAbs);
	  void ReadTextFileLines( const std::string& aPathAbs, std::vector<std::string>& someLinesOut);
    void ReadTextFileLines( const std::string& aPathAbs, std::list<std::string>& someLinesOut);
    bool ReadBinaryFile(const char* aPathAbs, DynamicArray<uint8>& someDataOut);
  };
//---------------------------------------------------------------------------//
}
