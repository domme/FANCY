#pragma once
#include "FixedArray.h"
#include <array>

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext;
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  const uint32 kMaxNumDescriptorSetElements = 32u;
//---------------------------------------------------------------------------//
  enum class SriElementType
  {
    Constants = 0,
    Descriptor,
    DescriptorSet
  };
  //---------------------------------------------------------------------------//
  enum class SriResourceType
  {
    ConstantBuffer = 0,
    BufferOrTexture,  // TODO (Important): Check if Vulkan needs to distinguish between buffer and texture on binding-level
    BufferOrTextureRW,
    Sampler,
    Float,
  };
//---------------------------------------------------------------------------//
  struct SriDescriptorSetElement
  {
    uint32 myNumElements;
    SriResourceType myResourceType;
    uint32 myBindingSlot;
  };
//---------------------------------------------------------------------------//
  struct SriDescriptorSet
  {
    uint32 myNumElements;
    SriDescriptorSetElement myRangeElements[kMaxNumDescriptorSetElements];
  };
//---------------------------------------------------------------------------//
  struct SriDescriptorElement
  {
    SriResourceType myResourceType;
    uint32 myBindingSlot;
  };
//---------------------------------------------------------------------------//
  struct SriConstantsElement
  {
    uint32 myBindingSlot;
    uint32 myNumValues;
  };
//---------------------------------------------------------------------------//
  struct SriElement
  {
    SriElementType myType;

    union
    {
      SriDescriptorSet myDescriptorSet;
      SriDescriptorElement myDescriptor;
      SriConstantsElement myConstants;
    };
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  struct ShaderResourceInterfaceDesc
  {
    
    std::vector<SriElement> myElements;
    uint64 myHash;
  };
//---------------------------------------------------------------------------//
} }  // Fancy::Rendering
