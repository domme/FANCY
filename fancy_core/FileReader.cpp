#include "fancy_core_precompile.h"

#include "FileReader.h"

namespace Fancy 
{
  namespace FileReader 
  {
//---------------------------------------------------------------------------//
    std::string ReadTextFile(const char* aPathAbs)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);
      std::stringstream stringStream;
      stringStream << fileStream.rdbuf();
      return stringStream.str();
    }
//---------------------------------------------------------------------------//
    void ReadTextFileLines(const char* aPathAbs, std::vector<std::string>& someLinesOut)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);

      if (fileStream.good())
      {
        std::string line;
        while (!fileStream.eof())
        {
          std::getline(fileStream, line);
          someLinesOut.push_back(line);
        }
      }
    }
    //---------------------------------------------------------------------------//
    void ReadTextFileLines(const char* aPathAbs, std::list<std::string>& someLinesOut)
    {
      std::ifstream fileStream;
      fileStream.open(aPathAbs);

      if (fileStream.good())
      {
        std::string line;
        while (!fileStream.eof())
        {
          std::getline(fileStream, line);
          someLinesOut.push_back(line);
        }
      }
    }
//---------------------------------------------------------------------------//
    bool ReadBinaryFile(const char* aPathAbs, DynamicArray<uint8>& someDataOut)
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