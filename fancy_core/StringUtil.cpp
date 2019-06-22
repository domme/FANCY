#include "fancy_core_precompile.h"
#include "StringUtil.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  std::wstring StringUtil::ToWideString(const String& aStr)
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(aStr);
  }
//---------------------------------------------------------------------------//
  void StringUtil::Tokenize(const String& _str, const char* _szDelimiters, std::list<String>& _outTokenList)
  {
    char* cstr = const_cast<char*>(_str.c_str());
    cstr = strtok(cstr, _szDelimiters);

    while (cstr != nullptr)
    {
      _outTokenList.push_back(cstr);
      cstr = strtok(nullptr, _szDelimiters);
    }
  }
//---------------------------------------------------------------------------//
}
