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
      uint len = aString == nullptr ? 0 : (uint) strlen(aString);
      if (len == 0)
        return;

      eastl::fixed_vector<char, 512> tempStrBuf(len + 1);
      char* tempStr = tempStrBuf.data();
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
