#pragma once

#include "Any.h"
#include "DynamicArray.h"

namespace Fancy {
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
    BufferOrTexture,
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
    enum { MAX_DESCRIPTOR_SET_ELEMENTS = 64 };
    uint myNumElements;
    SriDescriptorSetElement myRangeElements[MAX_DESCRIPTOR_SET_ELEMENTS];
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
  class ShaderResourceInterface
  {
  public:
    DynamicArray<SriElement> myElements;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}