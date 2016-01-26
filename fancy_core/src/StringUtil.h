#ifndef INCLUDE_STRINGUTIL_H
#define INCLUDE_STRINGUTIL_H

#include <list>
#include <codecvt>

#include "FancyCorePrerequisites.h"

namespace Fancy {

  class StringUtil
  {
  public:
//---------------------------------------------------------------------------//
    static std::wstring ToWideString(const String& aStr)
    {
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
      return converter.from_bytes(aStr);
    }
//---------------------------------------------------------------------------//
    template<class T>
    static std::string toString(const T& _val)
    {
      std::stringstream ss;
      ss << _val;
      return ss.str();
    }
//---------------------------------------------------------------------------//
    static void tokenize(const String& _str, const char* _szDelimiters, std::list<String>& _outTokenList )
    {
      // Create a temporary string we can "destroy" in the process
      String str = _str;
      char* cstr = const_cast<char*>(str.c_str());
      cstr = strtok(cstr, _szDelimiters);
      
      while(cstr != nullptr)
      {
        _outTokenList.push_back(cstr);
        cstr = strtok(NULL, _szDelimiters);
      }
    }
//---------------------------------------------------------------------------//
  };

}  // end of namespace Fancy


#endif  // INCLUDE_STRINGUTIL_H