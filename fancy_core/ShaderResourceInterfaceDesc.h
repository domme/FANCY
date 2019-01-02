#pragma once

#include "FancyCoreDefines.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class RenderContext;
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  const uint kMaxNumDescriptorSetElements = 32u;
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
    uint myNumElements;
    SriResourceType myResourceType;
    uint myBindingSlot;
  };
//---------------------------------------------------------------------------//
  struct SriDescriptorSet
  {
    uint myNumElements;
    SriDescriptorSetElement myRangeElements[kMaxNumDescriptorSetElements];
  };
//---------------------------------------------------------------------------//
  struct SriDescriptorElement
  {
    SriResourceType myResourceType;
    uint myBindingSlot;
  };
//---------------------------------------------------------------------------//
  struct SriConstantsElement
  {
    uint myBindingSlot;
    uint myNumValues;
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
    DynamicArray<SriElement> myElements;
    uint64 myHash = 0u;
  };
//---------------------------------------------------------------------------//
}
