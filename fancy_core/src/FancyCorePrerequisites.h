#ifndef INCLUDE_FANCYCOREPREREQUISITES_H
#define INCLUDE_FANCYCOREPREREQUISITES_H

// Disable some warnings...
#pragma warning( disable : 4251 )  // "...Needs to have a dll-interface to be used by clients"

//STD includes
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <Windows.h>
#include <memory>
#include <functional>

//Math includes
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//Common MACRO defines
#define SAFE_DELETE(p) if(p){ delete p; p = NULL; }
#define SAFE_DELETE_ARR(p) if( p[ 0 ] ) delete[] p;
#define BUFFER_OFFSET(i) ( (char*) NULL + (i) )
//---------------------------------------------------------------------------//  
  template<typename T>
  void _log_impl(T s, const std::string& szPrefix)
  {
    std::ostringstream ss;
    ss << szPrefix << s << '\n';

#ifdef __WINDOWS
    OutputDebugStringA( ss.str().c_str() );
#else
    std::cout << ss.str();
#endif
  };
//---------------------------------------------------------------------------//
  template<typename T>
  void log_Info( T s )
  {
    _log_impl(s, "Info: ");
  }
//---------------------------------------------------------------------------//
  template<typename T>
  void log_Warning( T s )
  {
    _log_impl(s, "Warning: ");
  }
//---------------------------------------------------------------------------//
  template<typename T>
  void log_Error( T s )
  {
    _log_impl(s, "ERROR: ");
  }
//---------------------------------------------------------------------------//
/* For some reason this specialization doesnt work...
void log_Info( glm::vec3 vec )
{
  std::stringstream ss;
  ss << vec.x << " " << vec.y << " " << vec.z;

  log_Info( ss.str() );
} */
//---------------------------------------------------------------------------//
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
//---------------------------------------------------------------------------//
#define DECLARE_SMART_PTRS(class) \
  typedef std::shared_ptr<##class> ##class##Ptr; \
  typedef std::weak_ptr<##class> ##class##WeakPtr;
//---------------------------------------------------------------------------//
  #define STATIC_ASSERT( condition, message ) \
  {								\
    static_assert(condition, message); \
  }
//---------------------------------------------------------------------------//
  #define ASSERT( value ) \
  { \
    if(!(value)) DebugBreak(); \
  }
//---------------------------------------------------------------------------//
  #define ASSERT_M( value, message ) \
  { \
    if(!(value)) { \
      log_Info(message); \
      DebugBreak(); \
    }\
  }
//---------------------------------------------------------------------------//
  #define RUN_NOT_FIRST( function ) \
  {								\
    static bool bFirst = false; \
                  \
    if( bFirst )  \
    {							\
      function;		\
    }							\
                  \
    bFirst = true; \
  }
//---------------------------------------------------------------------------//
  #define RUN_ONLY_ONCE_STATIC( function ) \
  {								\
    static bool bInit = false;	\
                  \
    if( bInit == true )			\
      return;					\
                  \
    function;					\
                  \
    bInit = true;				\
  }	
//---------------------------------------------------------------------------//
//DLL-Export MACROS
#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------//
// Functional defines
//-----------------------------------------------------------------------//
/// Enables various sanity-checks and validations
#define FANCY_RENDERSYSTEM_USE_VALIDATION
/// Enables the storage of strings in ObjectNames
#define FANCY_COMMON_USE_OBJECTNAME_STRINGS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Forward declarations
//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  class ObjectName;
//---------------------------------------------------------------------------//
  namespace Geometry {
    class Model;
    class SubModel;
    class Mesh;
    class GeometryData;
  }
//---------------------------------------------------------------------------//
} // end of namespace Fancy

//---------------------------------------------------------------------------//
// Typedefs
//---------------------------------------------------------------------------//
  namespace Fancy {
    typedef std::string String;

    typedef glm::uint16		uint16;
    typedef glm::uint32		uint32;
    typedef glm::uint64		uint64;
    typedef glm::uint8		uint8;
    typedef size_t		    uint; // TODO: Not sure if its a good idea to promote a standard uint to 64bit yet...
    // typedef glm::uint32     uint;
    typedef glm::int32    int32;
    typedef double        float64;

    // TODO: This seems to give compile-errors on VS 2012
    //template<class T>
    //using DynamicArray = std::vector<T>;
  }  // end of namespace Fancy
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//

#endif  // INCLUDE_FANCYCOREPREREQUISITES_H