#include "fancy_core_precompile.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  uint64 VertexInputLayoutProperties::GetHash() const
  {
    if (myAttributes.IsEmpty() && myBufferBindings.IsEmpty())
      return 0ull;

    MathUtil::BeginMultiHash();
    
    if (!myAttributes.IsEmpty())
      MathUtil::AddToMultiHash(myAttributes.GetBuffer(), myAttributes.ByteSize());
    if (!myBufferBindings.IsEmpty())
      MathUtil::AddToMultiHash(myBufferBindings.GetBuffer(), myBufferBindings.ByteSize());

    return MathUtil::EndMultiHash();
  }
//---------------------------------------------------------------------------//
  VertexInputLayout::VertexInputLayout(const VertexInputLayoutProperties& someProperties)
    : myProperties(someProperties)
  {
    // Check for validity and compute the buffer-offsets
    uint bufferSize[16] = { 0u };
    for (uint i = 0; i < myProperties.myAttributes.Size(); ++i)
    {
      VertexInputAttributeDesc& attributeDesc = myProperties.myAttributes[i];
      
      const uint bufferIndex = attributeDesc.myBufferIndex;

      ASSERT(bufferIndex < myProperties.myBufferBindings.Size());
      ASSERT(bufferIndex < ARRAY_LENGTH(bufferSize));

      myAttributeOffsetsInBuffer.Add(bufferSize[bufferIndex]);

      const uint size = DataFormatInfo::GetFormatInfo(attributeDesc.myFormat).mySizeBytes;
      bufferSize[attributeDesc.myBufferIndex] += size;
    }
  }
//---------------------------------------------------------------------------//
}
