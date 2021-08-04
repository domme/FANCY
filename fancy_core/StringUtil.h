#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  namespace StringUtil
  {
//---------------------------------------------------------------------------//
    eastl::wstring ToWideString(const char* aStr);
    eastl::wstring ToWideString(const eastl::string& aStr);
    eastl::string ToNarrowString(const std::wstring& aStr);
//---------------------------------------------------------------------------//
    template<class VectorT>
    void Tokenize(const char* aString, const char* someDelimiters, VectorT& someTokensOut)
    {
      uint len = strlen(aString);
      if (len == 0)
        return;

      char* tempStr = new char[len + 1];
      memcpy(tempStr, aString, len);
      tempStr[len] = '\0';

      tempStr = strtok(tempStr, someDelimiters);

      while (tempStr != nullptr)
      {
        someTokensOut.push_back(tempStr);
        tempStr = strtok(nullptr, someDelimiters);
      }
    }
//---------------------------------------------------------------------------//
    int FindFirstOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
    int FindLastOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
