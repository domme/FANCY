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
  String StringUtil::ToNarrowString(const std::wstring& aStr)
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(aStr);
  }
//---------------------------------------------------------------------------//
  void StringUtil::Tokenize(const String& _str, const char* _szDelimiters, DynamicArray<String>& _outTokenList)
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
  int StringUtil::FindFirstOf(const char* aStr, char aChar)
  {
    const uint len = (uint)strlen(aStr);

    for (uint i = 0u; i < len; ++i)
      if (aStr[i] == aChar)
        return (int)i;

    return -1;
  }
//---------------------------------------------------------------------------//
  int StringUtil::FindLastOf(const char* aStr, char aChar)
  {
    const uint len = (uint)strlen(aStr);
    for (int i = len - 1; i >= 0; --i)
      if (aStr[i] == aChar)
        return i;

    return -1;
  }
//---------------------------------------------------------------------------//
}
