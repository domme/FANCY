#include "fancy_core_precompile.h"
#include "TextureReadbackTask.h"

#include "RenderCore.h"
#include "GpuBuffer.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  ReadbackBufferAllocation::~ReadbackBufferAllocation()
  {
    if (myBuffer != nullptr && myBlockSize != 0u)
      RenderCore::FreeReadbackBuffer(myBuffer, myBlockSize, myOffsetToBlock);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void ReadbackTask::GetRawData(DynamicArray<uint8>& aDataOut)
  {
    ASSERT(IsCompleted());
    ASSERT(myBufferAllocation != nullptr);

    aDataOut.resize(myBufferAllocation->myBlockSize);

    const uint8* srcBufferData = static_cast<const uint8*>(
      myBufferAllocation->myBuffer->Map(GpuResourceMapMode::READ_UNSYNCHRONIZED,
        myBufferAllocation->myOffsetToBlock, myBufferAllocation->myBlockSize));
    ASSERT(srcBufferData != nullptr);

    memcpy(aDataOut.data(), srcBufferData, myBufferAllocation->myBlockSize);

    myBufferAllocation->myBuffer->Unmap(GpuResourceMapMode::READ_UNSYNCHRONIZED,
      myBufferAllocation->myOffsetToBlock, myBufferAllocation->myBlockSize);
  }
//---------------------------------------------------------------------------//
  bool ReadbackTask::IsCompleted() const
  {
    return RenderCore::IsFenceDone(myFence);
  }
//---------------------------------------------------------------------------//
  void ReadbackTask::Wait() const
  {
    RenderCore::WaitForFence(myFence);
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  void TextureReadbackTask::GetData(TextureData& aDataOut) const
  {
    ASSERT(IsCompleted());
    ASSERT(myBufferAllocation != nullptr);
  
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(myTextureProperties.myFormat);

    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    const uint64 rowAlignment = caps.myTextureRowAlignment;
    const uint64 subresourceAlignment = caps.myTextureSubresourceBufferAlignment;

    uint64 overallSizeBytes = 0u;
    uint64 expectedBufferSize = 0u;
    for (SubresourceIterator it = mySubresourceRange.Begin(), end = mySubresourceRange.End(); it != end; ++it)
    {
      const SubresourceLocation& subResource = *it;

      uint width, height, depth;
      myTextureProperties.GetSize(subResource.myMipLevel, width, height, depth);

      const uint64 rowSize = width * formatInfo.mySizeBytesPerPlane[subResource.myPlaneIndex];
      const uint64 alignedRowSize = MathUtil::Align(rowSize, rowAlignment);

      const uint64 subresourceSize = rowSize * height * depth;
      const uint64 alignedSubresourceSize = MathUtil::Align(alignedRowSize * height * depth, subresourceAlignment);

      overallSizeBytes += subresourceSize;
      expectedBufferSize += alignedSubresourceSize;
    }

    ASSERT(myBufferAllocation->myBlockSize == expectedBufferSize);

    aDataOut.myData.resize(overallSizeBytes);
    aDataOut.mySubDatas.resize(mySubresourceRange.GetNumSubresources());

    const uint8* srcSubResourceStart = static_cast<const uint8*>(
      myBufferAllocation->myBuffer->Map(GpuResourceMapMode::READ_UNSYNCHRONIZED,
      myBufferAllocation->myOffsetToBlock, myBufferAllocation->myBlockSize));
    ASSERT(srcSubResourceStart != nullptr);

    uint8* dstSubResourceStart = aDataOut.myData.data();
    uint subResourceIndex = 0u;
    for (SubresourceIterator it = mySubresourceRange.Begin(), end = mySubresourceRange.End(); it != end; ++it, ++subResourceIndex)
    {
      const SubresourceLocation& subResource = *it;
      uint width, height, depth;
      myTextureProperties.GetSize(subResource.myMipLevel, width, height, depth);

      const uint64 dstRowSizeBytes = width * formatInfo.mySizeBytesPerPlane[subResource.myPlaneIndex];
      const uint64 srcRowSizeBytes = MathUtil::Align(dstRowSizeBytes, rowAlignment);

      const uint64 srcSliceSizeBytes = srcRowSizeBytes * height;
      const uint64 dstSliceSizeBytes = dstRowSizeBytes * height;

      TextureSubData& dstSubData = aDataOut.mySubDatas[subResourceIndex];
      dstSubData.myData = dstSubResourceStart;
      dstSubData.myPixelSizeBytes = formatInfo.mySizeBytesPerPlane[subResource.myPlaneIndex];
      dstSubData.myRowSizeBytes = dstRowSizeBytes;
      dstSubData.mySliceSizeBytes = dstSliceSizeBytes;
      dstSubData.myTotalSizeBytes = dstSliceSizeBytes * depth;

      const uint64 srcSubresourceSizeBytes = srcSliceSizeBytes * depth;
      const uint64 dstSubresourceSizeBytes = MathUtil::Align(dstSliceSizeBytes * depth, caps.myTextureSubresourceBufferAlignment);

      const uint8* srcSliceStart = srcSubResourceStart;
      uint8* dstSliceStart = dstSubResourceStart;
      for (uint d = 0u; d < depth; ++d)
      {
        const uint8* srcRowStart = srcSliceStart;
        uint8* dstRowStart = dstSliceStart;
        for (uint h = 0u; h < height; ++h)
        {
          memcpy(dstRowStart, srcRowStart, dstRowSizeBytes);

          srcRowStart += srcRowSizeBytes;
          dstRowStart += dstRowSizeBytes;
        }

        srcSliceStart += srcSliceSizeBytes;
        dstSliceStart += dstSliceSizeBytes;
      }

      srcSubResourceStart += srcSubresourceSizeBytes;
      dstSubResourceStart += dstSubresourceSizeBytes;
    }

    myBufferAllocation->myBuffer->Unmap(GpuResourceMapMode::READ_UNSYNCHRONIZED, 
      myBufferAllocation->myOffsetToBlock, myBufferAllocation->myBlockSize);
  }
//---------------------------------------------------------------------------//
 
}
