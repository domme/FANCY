#pragma once

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
#include <array>
#include <malloc.h>
#include <stdio.h>

//Math includes
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//Common MACRO defines
#define SAFE_DELETE(p) if(p){ delete p; p = NULL; }
#define SAFE_DELETE_ARR(p) if( p[ 0 ] ) delete[] p;
#define BUFFER_OFFSET(i) ( (char*) NULL + (i) )

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
  template<class T, unsigned int Size>
  constexpr unsigned int GetArraySize(T(&anArray)[Size])
  {
    return Size;
  }
//---------------------------------------------------------------------------//
#define ARRAY_LENGTH(array) GetArraySize(array)
#define DYN_ARRAY_BYTESIZE(array) (array.size() * sizeof(decltype(array)::value_type))
  //---------------------------------------------------------------------------//
  template<class T>
  using SharedPtr = std::shared_ptr<T>;

  template<class T>
  using UniquePtr = std::unique_ptr<T>;
//---------------------------------------------------------------------------//
// Functional defines
//-----------------------------------------------------------------------//
/// Enables various sanity-checks and validations
#define FANCY_RENDERSYSTEM_USE_VALIDATION
/// Enables the storage of strings in ObjectNames
#define FANCY_COMMON_USE_OBJECTNAME_STRINGS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Typedefs
//---------------------------------------------------------------------------//
  typedef glm::uint16		uint16;
  typedef glm::uint64		uint64;
  typedef glm::uint8		uint8;
  typedef unsigned int	uint;
  typedef double        float64;

  template<class T>
  using DynamicArray = std::vector<T>;

  template<class T, size_t N>
  using FixedArray = std::array<T, N>;
//---------------------------------------------------------------------------//
  #include "FC_String.h"
  #include "Log.h"
//---------------------------------------------------------------------------//