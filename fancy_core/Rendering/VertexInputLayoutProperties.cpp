#include "fancy_core_precompile.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  uint64 VertexInputLayoutProperties::GetHash() const
  {
    if (myAttributes.empty() && myBufferBindings.empty())
      return 0ull;

    MathUtil::BeginMultiHash();

    MathUtil::AddToMultiHash(myAttributes.data(), VECTOR_BYTESIZE(myAttributes));
    MathUtil::AddToMultiHash(myBufferBindings.data(), VECTOR_BYTESIZE(myBufferBindings));

    return MathUtil::EndMultiHash();
  }
//---------------------------------------------------------------------------//
  uint64 VertexInputLayoutProperties::GetOverallVertexSize() const
  {
    uint64 overallSize = 0ull;
    for (const VertexInputAttributeDesc& attribute : myAttributes)
      overallSize += BITS_TO_BYTES(DataFormatInfo::GetFormatInfo(attribute.myFormat).myBitsPerPixel);

    return overallSize;
  }
//---------------------------------------------------------------------------//
  VertexInputLayout::VertexInputLayout(const VertexInputLayoutProperties& someProperties)
    : myProperties(someProperties)
  {
    // Check for validity and compute the buffer-offsets
    uint bufferSize[16] = { 0u };
    for (const VertexInputAttributeDesc& attributeDesc : myProperties.myAttributes)
    {
      const uint bufferIndex = attributeDesc.myBufferIndex;

      ASSERT(bufferIndex < myProperties.myBufferBindings.size());
      ASSERT(bufferIndex < ARRAY_LENGTH(bufferSize));

      myAttributeOffsetsInBuffer.push_back(bufferSize[bufferIndex]);

      const uint size = BITS_TO_BYTES(DataFormatInfo::GetFormatInfo(attributeDesc.myFormat).myBitsPerPixel);
      bufferSize[attributeDesc.myBufferIndex] += size;
    }
  }
//---------------------------------------------------------------------------//
}
