#pragma once

#include <list>
#include <string>

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
	  std::string ReadTextFile(const char* aPathAbs);
	  void ReadTextFileLines( const char* aPathAbs, eastl::vector<std::string>& someLinesOut);
    void ReadTextFileLines( const char* aPathAbs, std::list<std::string>& someLinesOut);
    bool ReadBinaryFile(const char* aPathAbs, eastl::vector<uint8>& someDataOut);
  };
//---------------------------------------------------------------------------//
}
