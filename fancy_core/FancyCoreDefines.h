#pragma once

#include <cstdint>
#include <climits>
#include <string>
#include <cinttypes>

//Common MACRO defines
#define SAFE_DELETE(p) if(p){ delete p; p = NULL; }
#define SAFE_DELETE_ARR(p) if( p[ 0 ] ) delete[] p;
#define BUFFER_OFFSET(i) ( (char*) NULL + (i) )
#define SIZE_MB (1024 * 1024)
#define SIZE_KB (1024)
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
  
//---------------------------------------------------------------------------//
// Typedefs
//---------------------------------------------------------------------------//
  using uint16 = std::uint16_t;
  using uint64 = std::uint64_t;
  using uint8 = std::uint8_t;
  using uint = unsigned int;
  using float64 = double; 
//---------------------------------------------------------------------------//
  namespace Fancy {
    using String = std::string;
  }
//---------------------------------------------------------------------------//
// Compile-switches for debugging and validation in all core- and common-files
//---------------------------------------------------------------------------//
#define CORE_DEBUG_MEMORY_ALLOCATIONS 1

#define FANCY_ENABLE_VK 1
#define FANCY_ENABLE_DX12 1



