#ifndef INCLUDE_GPUPROGRAMRESOURCE_H
#define INCLUDE_GPUPROGRAMRESOURCE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMRESOURCE

#include "ObjectName.h"
#include "FixedArray.h"
#include "Serializable.h"

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
  struct ConstantBufferElement
  {
    SERIALIZABLE(ConstantBufferElement)
    void serialize(IO::Serializer* aSerializer);
    const ObjectName& getName() const { return ObjectName::blank; }
    ObjectName getTypeName() const { return _N(ConstantBufferElement); }

    ConstantBufferElement() :
      uOffsetBytes(0u), uSizeBytes(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    ObjectName name;
    uint32 uOffsetBytes;  // Byte-offset from the start of the buffer
    uint32 uSizeBytes;  // Overall size of the element (==sizeof(eFormat) * uFormatComponentCount)
    DataFormat eFormat;
    uint8 uFormatComponentCount;  // Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
  };
//---------------------------------------------------------------------------//
  /// Describes a resource (texture, buffer, ...) used in a gpuProgram as returned from reflection
  struct GpuProgramResourceInfo : public PLATFORM_DEPENDENT_NAME(GpuProgramResource) {
    GpuProgramResourceInfo()
      : u32RegisterIndex(0u),
        eAccessType(GpuResourceAccessType::READ_ONLY), 
        eResourceType(GpuResourceType::NONE) {}

    SERIALIZABLE(GpuProgramResourceInfo)
    void serialize(IO::Serializer* aSerializer);
    const ObjectName& getName() const { return ObjectName::blank; }
    ObjectName getTypeName() const { return _N(GpuProgramResourceInfo); }

    uint32 u32RegisterIndex;
    ObjectName name;
    GpuResourceAccessType eAccessType;
    GpuResourceType eResourceType;
  };
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
 typedef std::vector<ConstantBufferElement> ConstantBufferElementList;
 typedef FixedArray<GpuProgramResourceInfo, kMaxNumGpuProgramResources> GpuResourceInfoList;
 typedef FixedArray<Texture*, kMaxNumGpuProgramResources> GpuTextureResourceList;
 typedef FixedArray<GpuBuffer*, kMaxNumGpuProgramResources> GpuBufferResourceList;
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAMRESOURCE_H