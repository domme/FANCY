#pragma once

#include <crtdbg.h>

// Required by EASTL
inline void* operator new[](size_t size, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* file, int line)
{
#if _DEBUG
  return _aligned_malloc_dbg(size, 1, file, line);
#else
  return _aligned_malloc(size, 1);
#endif
}

inline void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* file, int line)
{
#if _DEBUG
  return _aligned_offset_malloc_dbg(size, alignment, alignmentOffset, file, line);
#else
  return _aligned_offset_malloc(size, alignment, alignmentOffset);
#endif
}
