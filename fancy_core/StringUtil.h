#pragma once

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
    String ToNarrowString(const std::wstring& aStr);
//---------------------------------------------------------------------------//
    void Tokenize(const String& _str, const char* _szDelimiters, std::list<String>& _outTokenList);
//---------------------------------------------------------------------------//
    int FindFirstOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
    int FindLastOf(const char* aStr, char aChar);
//---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
