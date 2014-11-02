#ifndef INCLUDE_GPUPROGRAMRESOURCE_H
#define INCLUDE_GPUPROGRAMRESOURCE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMRESOURCE

#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class GpuResourceAccessType {
    READ_ONLY = 0,
    READ_WRITE
  };
//---------------------------------------------------------------------------//
  enum class GpuResourceType {
    NONE = 0,
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_CUBE,
    TEXTURE_1D_SHADOW,
    TEXTURE_2D_SHADOW,
    TEXTURE_CUBE_SHADOW,
    BUFFER_TEXTURE,
    BUFFER,
    SAMPLER,

    NUM
  };
//---------------------------------------------------------------------------//
  /// Describes a resource (texture, buffer, ...) used in a gpuProgram as returned from reflection
  struct GpuProgramResourceInfo : public PLATFORM_DEPENDENT_NAME(GpuProgramResource) {
    GpuProgramResourceInfo()
      : u32RegisterIndex(0u),
        eAccessType(GpuResourceAccessType::READ_ONLY), 
        eResourceType(GpuResourceType::NONE) {}

    uint32 u32RegisterIndex;
    ObjectName name;
    GpuResourceAccessType eAccessType;
    GpuResourceType eResourceType;
  };
//---------------------------------------------------------------------------//
 typedef FixedArray<GpuProgramResourceInfo, kMaxNumGpuProgramResources> GpuResourceInfoList;
 typedef FixedArray<Texture*, kMaxNumGpuProgramResources> GpuTextureResourceList;
 typedef FixedArray<GpuBuffer*, kMaxNumGpuProgramResources> GpuBufferResourceList;
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAMRESOURCE_H