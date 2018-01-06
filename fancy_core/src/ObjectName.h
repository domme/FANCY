#ifndef INCLUDE_OBJECTNAME_H
#define INCLUDE_OBJECTNAME_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ObjectName 
  {
  //---------------------------------------------------------------------------//
    public:
      ObjectName();
      ObjectName(const std::string& szString);
      ObjectName(const char* aString);
      ObjectName(uint aHash);
      ~ObjectName();
      static const ObjectName blank;
    //---------------------------------------------------------------------------//
      std::string toString() const;
      uint getHash() const {return m_uNameHash;}
    //---------------------------------------------------------------------------//
      void operator=(const ObjectName& clOther);
      void operator=(const std::string& szOther);
      void operator=(const char* aString);
      bool operator==(const ObjectName& clOther) const;
      bool operator!=(const ObjectName& clOther) const;
      bool operator<(const ObjectName& clOther) const;
      /// Implicit conversion operator to size_t:
      operator size_t() const {return getHash();}
      operator std::string() const { return toString(); }
    //---------------------------------------------------------------------------//
    private:
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
      std::string m_szName;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
      uint   m_uNameHash;
  };
//---------------------------------------------------------------------------//
  std::string operator+(const std::string& szString, const ObjectName& name);
  std::string operator+(const ObjectName& name, const std::string& szString);
//---------------------------------------------------------------------------//
  bool operator==(const std::string& szOther, const ObjectName& name);
  bool operator==(const ObjectName& name, const std::string& szOther);
//---------------------------------------------------------------------------//
  bool operator!=(const std::string& szString, const ObjectName& name);
  bool operator!=(const ObjectName& name, const std::string& szString);
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
} // end of namespace Fancy

//---------------------------------------------------------------------------//
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
  #define _N(name) Fancy::ObjectName(#name)
#else // !FANCY_COMMON_USE_OBJECTNAME_STRINGS
  #define _N(name) Fancy::ObjectName(ObjectNameValues::name)
#endif // FANCY_COMMON_USE_OBJECTNAME_STRINGS
//---------------------------------------------------------------------------//
#endif  // INCLUDE_OBJECTNAME_H