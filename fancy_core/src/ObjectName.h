#ifndef INCLUDE_OBJECTNAME_H
#define INCLUDE_OBJECTNAME_H

#include "FancyCorePrerequisites.h"

namespace FANCY { namespace Core {
//---------------------------------------------------------------------------//
  class ObjectName {
    //---------------------------------------------------------------------------//
    public:
      ObjectName();
      ObjectName(const String& szString);
      ~ObjectName();
    //---------------------------------------------------------------------------//
      String toString();
    //---------------------------------------------------------------------------//
      void operator=(const ObjectName& clOther);
      void operator=(const String& szOther);
    //---------------------------------------------------------------------------//
      bool operator==(const ObjectName& clOther);
      bool operator==(const String& szOther);
    //---------------------------------------------------------------------------//
      bool operator!=(const ObjectName& clOther);
      bool operator!=(const String& clOther);
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
} } // end of namespace FANCY::Core

//---------------------------------------------------------------------------//
#define _N(name) FANCY::Core::ObjectName(name)
//---------------------------------------------------------------------------//
#endif  // INCLUDE_OBJECTNAME_H