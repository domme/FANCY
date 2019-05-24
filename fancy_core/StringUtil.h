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

  template<uint BufferSize>
  struct StrFmt
  {
    explicit StrFmt(const char* aFormatString, ...)
    {
      va_list args;
      va_start(args, aFormatString);

      const int neededSize = vsnprintf(nullptr, 0u, aFormatString, args) + 1;
      ASSERT(neededSize < (int)BufferSize);
      const int offset = vsnprintf(myBuffer, static_cast<size_t>(BufferSize), aFormatString, args);
      myBuffer[offset + 1] = '\0';
      va_end(args);
    }

    operator const char*() const { return myBuffer; }

    char myBuffer[BufferSize];
  };

}  // end of namespace Fancy
