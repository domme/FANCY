#pragma once

#include "FancyCorePrerequisites.h"

#include <string>
#include <sstream>

namespace Fancy {
  //---------------------------------------------------------------------------//
  inline void Format_Internal(const char* aFormat, std::stringstream& aStringStream)
  {
    aStringStream << aFormat;
  }
  //---------------------------------------------------------------------------//
  template<class T1, class...TArgs>
  void Format_Internal(const char* aFormat, std::stringstream& aStringStream, const T1& aValue, const TArgs&... FArgs)
  {
    for (; *aFormat != '\0'; ++aFormat)
    {
      if (*aFormat == '%')
      {
        aStringStream << aValue;
        Format_Internal(aFormat + 1, aStringStream, FArgs...);
        return;
      }
      aStringStream << *aFormat;
    }
  }
  //---------------------------------------------------------------------------//
  class String : public std::string
  {
  public:
    String();
    String(const char* aStr);
    String(const std::string& aStr);
    ~String();

    template<class...TArgs>
    void Format(const char* aFormat, const TArgs&... FArgs)
    {
      std::stringstream ss;
      Format_Internal(aFormat, ss, FArgs...);
      assign(ss.str());
    }
  };
  //---------------------------------------------------------------------------//
  template<class...TArgs>
  String StringFormat(const char* aFormat, const TArgs&... FArgs)
  {
    std::stringstream ss;
    Format_Internal(aFormat, ss, FArgs...);
    return ss.str();
  }
  //---------------------------------------------------------------------------//
}  // end of namespace Fancy





