#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"
#include "EASTL/list.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace FileReader
  {
    eastl::string ReadTextFile(const char* aPathAbs);
	  void ReadTextFileLines( const char* aPathAbs, eastl::vector<eastl::string>& someLinesOut);
    void ReadTextFileLines( const char* aPathAbs, eastl::list<eastl::string>& someLinesOut);
    bool ReadBinaryFile(const char* aPathAbs, eastl::vector<uint8>& someDataOut);
  };
//---------------------------------------------------------------------------//
}
