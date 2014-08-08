#ifndef INCLUDE_GPUPROGRAMRESOURCE_H
#define INCLUDE_GPUPROGRAMRESOURCE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class GpuResourceAccessType {
    READ_ONLY = 0,
    WRITE_ONLY,
    READ_WRITE
  };
//---------------------------------------------------------------------------//
  /// TODO: Make this platform-dependent?
  /// Describes a resource (texture, buffer, ...) used in a gpuProgram as returned from reflection
  struct GpuProgramResourceInfo {
    uint32 u32RegisterIndex;
    uint32 u32Type; // represents platform-dependent typtes (e.g. GL_IMAGE_2D, GL_FLOAT_2, ...)
    ObjectName name;
    GpuResourceAccessType eAccessType;
  };
//---------------------------------------------------------------------------//
 typedef FixedArray<GpuProgramResourceInfo, kMaxNumGpuProgramResources> GpuResourceInfoList;
 typedef FixedArray<Texture*, kMaxNumGpuProgramResources> GpuTextureResourceList;
 typedef FixedArray<GpuBuffer*, kMaxNumGpuProgramResources> GpuBufferResourceList;
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Core::Rendering

#endif  // INCLUDE_GPUPROGRAMRESOURCE_H