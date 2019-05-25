#pragma once

#include "FC_String.h"
#include <list>

namespace Fancy 
{
//---------------------------------------------------------------------------//
  namespace StringUtil
  {
    template<class T>
    static String toString(const T& _val)
    {
      std::stringstream ss;
      ss << _val;
      return ss.str();
    }
//---------------------------------------------------------------------------//
    std::wstring ToWideString(const String& aStr);
//---------------------------------------------------------------------------//
    void Tokenize(const String& _str, const char* _szDelimiters, std::list<String>& _outTokenList);
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
