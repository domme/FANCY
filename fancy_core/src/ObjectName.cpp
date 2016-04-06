#include "ObjectName.h"

#include "MathUtil.h"
#include "StringUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace internal 
  {
    uint hashFromName(const std::string& szName) 
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

//---------------------------------------------------------------------------//
  const ObjectName ObjectName::blank("");
//---------------------------------------------------------------------------//
  ObjectName::ObjectName() :
      m_uNameHash(0)
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    , m_szName("0x0")
#endif // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  {

  }
//---------------------------------------------------------------------------//
  ObjectName::ObjectName( const std::string& szString )
  {
    m_uNameHash = internal::hashFromName(szString);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = szString;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  ObjectName::ObjectName(const char* aString)
  {
    m_uNameHash = internal::hashFromName(aString);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = aString;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  ObjectName::ObjectName(uint32 aHash)
  {
    m_uNameHash = aHash;
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = StringUtil::toString(aHash);
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  ObjectName::~ObjectName()
  {

  }
//---------------------------------------------------------------------------//
  std::string ObjectName::toString() const
  {
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    return m_szName;
#else
    // TODO: Check if that compiles...
    char cbuf[17];
    printf(cbuf, "0x80%u", m_uNameHash);
    return std::string(cbuf);
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
  void ObjectName::operator=( const std::string& szOther )
  {
    m_uNameHash = internal::hashFromName(szOther);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = szOther;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  void ObjectName::operator=(const char* aString)
  {
    m_uNameHash = internal::hashFromName(aString);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = aString;
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
  }
//---------------------------------------------------------------------------//
  bool ObjectName::operator==( const ObjectName& clOther ) const
  {
    return clOther.m_uNameHash == m_uNameHash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool ObjectName::operator!=( const ObjectName& clOther ) const
  {
    return !(*this == clOther);
  }
//---------------------------------------------------------------------------//
  bool ObjectName::operator<( const ObjectName& clOther ) const
  {
    return m_uNameHash < clOther.m_uNameHash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  std::string operator+(const std::string& szString, const ObjectName& name )
  {
    return szString + name.toString();
  }
//---------------------------------------------------------------------------//
  std::string operator+( const ObjectName& name, const std::string& szString )
  {
    return name.toString() + szString;
  }
//---------------------------------------------------------------------------//
  bool operator==(const std::string& szString, const ObjectName& name)
  {
    uint hash = internal::hashFromName(szString);
    return hash == name.getHash(); 
  }
//---------------------------------------------------------------------------//
  bool operator==( const ObjectName& name, const std::string& szString )
  {
    return szString == name; 
  }
//---------------------------------------------------------------------------//
  bool operator!=( const std::string& szString, const ObjectName& name )
  {
    return !(szString == name);
  }
//---------------------------------------------------------------------------//
  bool operator!=( const ObjectName& name, const std::string& szString )
  {
    return !(szString == name);
  }
//---------------------------------------------------------------------------//

} // end of namespace Fancy { namespace Common {
