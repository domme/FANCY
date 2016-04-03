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
#include <float.h>
#include <stdio.h>

//Math includes
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//Common MACRO defines
#define SAFE_DELETE(p) if(p){ delete p; p = NULL; }
#define SAFE_DELETE_ARR(p) if( p[ 0 ] ) delete[] p;
#define BUFFER_OFFSET(i) ( (char*) NULL + (i) )
#include <stdio.h>

//---------------------------------------------------------------------------//  
  enum class MemoryCategory {
    GENERAL,
    MATERIALS,
    TEXTURES,
    BUFFERS,
    GEOMETRY
  };
//---------------------------------------------------------------------------//
  // Allocation defines (will be replaced by custom allocators in the future)
  #define FANCY_NEW(type, memoryCategory) new type
  #define FANCY_DELETE(type, memoryCategory) delete type
  #define FANCY_DELETE_ARR(type, memoryCategory) delete[] type
  #define FANCY_ALLOCATE(sizeBytes, memoryCategory) malloc(sizeBytes)
  #define FANCY_FREE(pData, memoryCategory) free(pData)
//---------------------------------------------------------------------------//
  void Log(const char* aSeverity, const char* aFile, const int aLine, const char* aMessageFormat, ...)
  {
    va_list args;
    va_start(args, aMessageFormat);
    const int severitySizeBytes = strlen(aSeverity) + 2; // accounts for ": " between severity and message
    const int messageSizeBytes = severitySizeBytes + vsnprintf(nullptr, 0u, aMessageFormat, args) + 1u;

    char* messageBuf = (char*)alloca(messageSizeBytes);



    snprintf(messageBuf, severitySizeBytes, "%s: ", aSeverity);
    sprintf(messageBuf + severitySizeBytes, aMessageFormat, args);

    va_end(args);

    OutputDebugStringA(messageBuf);
  }
  //---------------------------------------------------------------------------//
#define LOG_INFO(aFormat, ...) Log("Info", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)

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
    typedef size_t		    uint;
    typedef glm::int32    int32;
    typedef double        float64;

    // TODO: This seems to give compile-errors on VS 2012
    //template<class T>
    //using DynamicArray = std::vector<T>;
  }  // end of namespace Fancy
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//

#endif  // INCLUDE_FANCYCOREPREREQUISITES_H