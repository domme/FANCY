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
#include <windef.h>

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
  template<class T, int Size>
  constexpr int GetArraySize(T(&anArray)[Size])
  {
    return Size;
  }
//---------------------------------------------------------------------------//
#define ARRAY_LENGTH(array) GetArraySize(array)
//---------------------------------------------------------------------------//
  template<class T>
  using SharedPtr = std::shared_ptr<T>;
//---------------------------------------------------------------------------//
#define DECLARE_SMART_PTRS(class) \
  typedef std::shared_ptr<##class> ##class##Ptr; \
  typedef std::weak_ptr<##class> ##class##WeakPtr;
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
  #include "FC_String.h"
  #include "Log.h"
//---------------------------------------------------------------------------//

#endif  // INCLUDE_FANCYCOREPREREQUISITES_H