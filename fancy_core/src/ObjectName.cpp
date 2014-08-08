#include "ObjectName.h"

#include "MathUtil.h"

namespace Fancy { namespace Core {
//---------------------------------------------------------------------------//
  namespace internal 
  {
    uint hashFromName(const String& szName) 
    {
      if (szName != "0x0") {
        return MathUtil::hashFromString(szName);
      }
      else {
        return 0;
      }
    }
  }
//---------------------------------------------------------------------------//
  ObjectName::ObjectName() :
      m_uNameHash(0)
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    , m_szName("0x0")
#endif // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  {

  }
//---------------------------------------------------------------------------//
  ObjectName::ObjectName( const String& szString )
  {
    m_uNameHash = internal::hashFromName(szString);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = szString;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  ObjectName::~ObjectName()
  {

  }
//---------------------------------------------------------------------------//
  String ObjectName::toString() const
  {
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    return m_szName;
#else
    // TODO: Check if that compiles...
    char cbuf[17];
    printf(cbuf, "0x80%u", m_uNameHash);
    return String(cbuf);
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  void ObjectName::operator=( const ObjectName& clOther )
  {
    m_uNameHash = clOther.m_uNameHash;

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = clOther.m_szName;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  void ObjectName::operator=( const String& szOther )
  {
    m_uNameHash = internal::hashFromName(szOther);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = szOther;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  bool ObjectName::operator==( const ObjectName& clOther )
  {
    return clOther.m_uNameHash == m_uNameHash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool ObjectName::operator!=( const ObjectName& clOther )
  {
    return !(*this == clOther);
  }
//---------------------------------------------------------------------------//
  bool ObjectName::operator<( const ObjectName& clOther )
  {
    return m_uNameHash < clOther.m_uNameHash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  String operator+(const String& szString, const ObjectName& name )
  {
    return szString + name.toString();
  }
//---------------------------------------------------------------------------//
  String operator+( const ObjectName& name, const String& szString )
  {
    return name.toString() + szString;
  }
//---------------------------------------------------------------------------//
  bool operator==(const String& szString, const ObjectName& name)
  {
    uint hash = internal::hashFromName(szString);
    return hash == name.getHash(); 
  }
//---------------------------------------------------------------------------//
  bool operator==( const ObjectName& name, const String& szString )
  {
    return szString == name; 
  }
//---------------------------------------------------------------------------//
  bool operator!=( const String& szString, const ObjectName& name )
  {
    return !(szString == name);
  }
//---------------------------------------------------------------------------//
  bool operator!=( const ObjectName& name, const String& szString )
  {
    return !(szString == name);
  }
//---------------------------------------------------------------------------//

} } // end of namespace Fancy { namespace Core { namespace Common {
