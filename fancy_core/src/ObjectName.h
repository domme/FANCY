#ifndef INCLUDE_OBJECTNAME_H
#define INCLUDE_OBJECTNAME_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Core {
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
    //---------------------------------------------------------------------------//
      bool operator==(const ObjectName& clOther);
    //---------------------------------------------------------------------------//
      bool operator!=(const ObjectName& clOther);
    //---------------------------------------------------------------------------//
      bool operator<(const ObjectName& clOther);
    //---------------------------------------------------------------------------//
    private:
  #if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
      String m_szName;
  #endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
      uint   m_uNameHash;
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
} } // end of namespace Fancy::Core



//---------------------------------------------------------------------------//
#define _N(name) Fancy::Core::ObjectName(name)
//---------------------------------------------------------------------------//
#endif  // INCLUDE_OBJECTNAME_H