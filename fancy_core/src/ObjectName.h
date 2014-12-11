#ifndef INCLUDE_OBJECTNAME_H
#define INCLUDE_OBJECTNAME_H

#include "FancyCorePrerequisites.h"

namespace Fancy { 
 //---------------------------------------------------------------------------//
  class ObjectName {
    //---------------------------------------------------------------------------//
    public:
      ObjectName();
      ObjectName(const String& szString);
      ~ObjectName();
    //---------------------------------------------------------------------------//
      String toString() const;
      uint getHash() const {return m_uNameHash;}
    //---------------------------------------------------------------------------//
      void operator=(const ObjectName& clOther);
      void operator=(const String& szOther);
      bool operator==(const ObjectName& clOther) const;
      bool operator!=(const ObjectName& clOther) const;
      bool operator<(const ObjectName& clOther) const;
      // Implicit conversion operator:
      operator size_t() const {return getHash();}
    //---------------------------------------------------------------------------//
    private:
      uint   m_uNameHash;
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
      String m_szName;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  };
//---------------------------------------------------------------------------//
  String operator+(const String& szString, const ObjectName& name);
  String operator+(const ObjectName& name, const String& szString);
//---------------------------------------------------------------------------//
  bool operator==(const String& szOther, const ObjectName& name);
  bool operator==(const ObjectName& name, const String& szOther);
//---------------------------------------------------------------------------//
  bool operator!=(const String& szString, const ObjectName& name);
  bool operator!=(const ObjectName& name, const String& szString);
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