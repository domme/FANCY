#include "fancy_core_precompile.h"
#include "StringUtil.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  eastl::wstring StringUtil::ToWideString(const char* aStr)
  {
    return eastl::wstring(eastl::wstring::CtorConvert(), aStr);
  }
//---------------------------------------------------------------------------//
  eastl::wstring StringUtil::ToWideString(const eastl::string& aStr)
  {
    return ToWideString(aStr.c_str());
  }
//---------------------------------------------------------------------------//
  eastl::string StringUtil::ToNarrowString(const std::wstring& aStr)
  {
    return eastl::string(eastl::string::CtorConvert(), aStr.c_str());
  }
//---------------------------------------------------------------------------//
  void StringUtil::Tokenize(const eastl::string& _str, const char* _szDelimiters, eastl::vector<eastl::string>& _outTokenList)
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
