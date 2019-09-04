#pragma once

#include <list>
#include <vector>
#include <string>

#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
	  std::string ReadTextFile(const char* aPathAbs);
	  void ReadTextFileLines( const char* aPathAbs, std::vector<std::string>& someLinesOut);
    void ReadTextFileLines( const char* aPathAbs, std::list<std::string>& someLinesOut);
    bool ReadBinaryFile(const char* aPathAbs, DynamicArray<uint8>& someDataOut);
  };
//---------------------------------------------------------------------------//
}
