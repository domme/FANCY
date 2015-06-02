#include "ObjectName.h"

#include "MathUtil.h"
#include "StringUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  void ShortStringDesc::operator=(const String& _someString)
  {
    ASSERT(kLength > _someString.length());
    std::copy(_someString.begin(), _someString.end(), &myChars[0]);
    myChars[_someString.length()] = 0u;  // Null-terminator
  }
//---------------------------------------------------------------------------//
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
  ObjectName::ObjectName( const String& szString )
  {
    m_uNameHash = internal::hashFromName(szString);

#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = szString;
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
  ObjectNameDesc ObjectName::getDescription() const
  {
    ObjectNameDesc aDesc;
    aDesc.myName = toString();
    aDesc.myHash = getHash();
    return aDesc;
  }
//---------------------------------------------------------------------------//
  void ObjectName::initFromDescription(const ObjectNameDesc someDesc)
  {
    m_uNameHash = someDesc.myHash;
#if defined (FANCY_COMMON_USE_OBJECTNAME_STRINGS)
    m_szName = someDesc.myName.toString();
#endif  // FANCY_COMMON_USE_OBJECTNAME_STRINGS
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

} // end of namespace Fancy { namespace Common {
