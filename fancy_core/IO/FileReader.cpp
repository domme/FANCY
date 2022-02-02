#include "fancy_core_precompile.h"

#include "FileReader.h"

#include <sstream>
#include <string>

namespace Fancy 
{
  namespace FileReader 
  {
//---------------------------------------------------------------------------//
    eastl::string ReadTextFile(const char* aPathAbs)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);
      std::stringstream stringStream;
      stringStream << fileStream.rdbuf();
      return eastl::string(stringStream.str().c_str());
    }
//---------------------------------------------------------------------------//
    void ReadTextFileLines(const char* aPathAbs, eastl::vector<eastl::string>& someLinesOut)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);

      if (fileStream.good())
      {
        std::string line;
        while (!fileStream.eof())
        {
          std::getline(fileStream, line);
          someLinesOut.push_back(eastl::string(line.c_str()));
        }
      }
    }
    //---------------------------------------------------------------------------//
    void ReadTextFileLines(const char* aPathAbs, eastl::list<eastl::string>& someLinesOut)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);

      if (fileStream.good())
      {
        std::string line;
        while (!fileStream.eof())
        {
          std::getline(fileStream, line);
          someLinesOut.push_back(eastl::string(line.c_str()));
        }
      }
    }
//---------------------------------------------------------------------------//
    bool ReadBinaryFile(const char* aPathAbs, eastl::vector<uint8>& someDataOut)
    {
      std::ifstream fileStream(aPathAbs, std::ios::ate | std::ios::binary);
      if (!fileStream.is_open())
        return false;

      const size_t fileSize = (size_t)fileStream.tellg();
      if (fileSize == 0)
        return false;

      someDataOut.resize(fileSize);
      fileStream.seekg(0);
      fileStream.read((char*)someDataOut.data(), fileSize);
      return true;
    }
//---------------------------------------------------------------------------//
  }
}
