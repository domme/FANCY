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
    void Tokenize(const eastl::string& _str, const char* _szDelimiters, eastl::vector<eastl::string>& _outTokenList);
//---------------------------------------------------------------------------//
    int FindFirstOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
    int FindLastOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
