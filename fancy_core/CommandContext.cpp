#include "CommandContext.h"
#include "RenderCore.h"
#include "MathUtil.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "GpuProgramPipeline.h"
#include "Texture.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "RenderPlatformCaps.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GraphicsPipelineState::GraphicsPipelineState()
    : myFillMode(FillMode::SOLID)
    , myCullMode(CullMode::BACK)
    , myWindingOrder(WindingOrder::CCW)
    , myNumRenderTargets(0u)
    , myDSVformat(DataFormat::UNKNOWN)
    , myIsDirty(true)
  {
    myDepthStencilState = RenderCore::GetDefaultDepthStencilState();
    myBlendState = RenderCore::GetDefaultBlendState();
  }
//---------------------------------------------------------------------------//
  uint64 GraphicsPipelineState::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, static_cast<uint>(myFillMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myCullMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myWindingOrder));
    MathUtil::hash_combine(hash, myDepthStencilState->GetHash());
    MathUtil::hash_combine(hash, myBlendState->GetHash());
    MathUtil::hash_combine(hash, myGpuProgramPipeline->GetHash());

    if (myGpuProgramPipeline != nullptr)
      MathUtil::hash_combine(hash, myGpuProgramPipeline->GetShaderByteCodeHash());

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < Constants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  CommandContext::CommandContext(CommandListType aType)
    : myCommandListType(aType)
    , myViewportParams(0, 0, 1, 1)
    , myClipRect(0, 0, 1, 1)
    , myViewportDirty(true)
    , myClipRectDirty(true)
    , myTopologyDirty(true)
    , myRenderTargetsDirty(true)
    , myDepthStencilTarget(nullptr)
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  ComputePipelineState::ComputePipelineState()
    : myIsDirty(true)
    , myGpuProgram(nullptr)
  {
  }
  //---------------------------------------------------------------------------//
  uint64 ComputePipelineState::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myGpuProgram));

    if (myGpuProgram != nullptr)
      MathUtil::hash_combine(hash, myGpuProgram->GetNativeBytecodeHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  void CommandContext::TransitionResource(GpuResource* aResource, GpuResourceState aTransitionToState)
  {
    GpuResource* resources[] = { aResource };
    GpuResourceState states[] = { aTransitionToState };
    TransitionResourceList(resources, states, ARRAY_LENGTH(resources));
  }
//---------------------------------------------------------------------------//
  void CommandContext::TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1, GpuResource* aResource2, GpuResourceState aTransitionToState2)
  {
    GpuResource* resources[] = { aResource1, aResource2 };
    GpuResourceState states[] = { aTransitionToState1, aTransitionToState2 };
    TransitionResourceList(resources, states, ARRAY_LENGTH(resources));
  }
//---------------------------------------------------------------------------//
  void CommandContext::TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1, GpuResource* aResource2, GpuResourceState aTransitionToState2, GpuResource* aResource3, GpuResourceState aTransitionToState3)
  {
    GpuResource* resources[] = { aResource1, aResource2, aResource3 };
    GpuResourceState states[] = { aTransitionToState1, aTransitionToState2, aTransitionToState3 };
    TransitionResourceList(resources, states, ARRAY_LENGTH(resources));
  }
//---------------------------------------------------------------------------//
  void CommandContext::TransitionResource(GpuResource* aResource1, GpuResourceState aTransitionToState1, GpuResource* aResource2, GpuResourceState aTransitionToState2, GpuResource* aResource3, GpuResourceState aTransitionToState3, GpuResource* aResource4, GpuResourceState aTransitionToState4)
  {
    GpuResource* resources[] = { aResource1, aResource2, aResource3, aResource4 };
    GpuResourceState states[] = { aTransitionToState1, aTransitionToState2, aTransitionToState3, aTransitionToState4 };
    TransitionResourceList(resources, states, ARRAY_LENGTH(resources));
  }
//---------------------------------------------------------------------------//
  const GpuBuffer* CommandContext::GetBuffer(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint aDataSize)
  {
    DynamicArray<GpuRingBuffer*>* ringBufferList = nullptr;
    uint64 sizeStep = 2 * SIZE_MB;
    
    if (aType == GpuBufferUsage::STAGING_UPLOAD)
    {
      ringBufferList = &myUploadRingBuffers;
      sizeStep = 2 * SIZE_MB;
    }
    else if (aType == GpuBufferUsage::CONSTANT_BUFFER)
    {
      ringBufferList = &myConstantRingBuffers;
      sizeStep = 2 * SIZE_MB;
    }
    else
    {
      ASSERT(false, "Not implemented!");
      return nullptr;
    }

    if (ringBufferList->empty() || ringBufferList->back()->GetFreeDataSize() < aDataSize)
      ringBufferList->push_back(RenderCore::AllocateRingBuffer(aType, MathUtil::Align(aDataSize, sizeStep)));

    GpuRingBuffer* ringBuffer = ringBufferList->back();
    uint offset = 0; 
    bool success = true;
    if (someData != nullptr)
      success = ringBuffer->AllocateAndWrite(someData, aDataSize, offset);
    else
      success = ringBuffer->Allocate(aDataSize, offset);
    ASSERT(success);
    
    anOffsetOut = offset;
    return ringBuffer->GetBuffer();
  }
//---------------------------------------------------------------------------//
  void CommandContext::BindConstantBuffer(void* someData, uint aDataSize, uint aRegisterIndex)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::CONSTANT_BUFFER, someData, aDataSize);
    
    BindResource(buffer, DescriptorType::CONSTANT_BUFFER, aRegisterIndex, offset);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Render Context:
//---------------------------------------------------------------------------//
  void CommandContext::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    ASSERT(myCommandListType == CommandListType::Graphics);

    if (myGraphicsPipelineState.myGpuProgramPipeline != aGpuProgramPipeline)
    {
      myGraphicsPipelineState.myGpuProgramPipeline = aGpuProgramPipeline;
      myGraphicsPipelineState.myIsDirty = true;
    }
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetComputeProgram(const GpuProgram* aProgram)
  {
    ASSERT(aProgram->myStage == ShaderStage::COMPUTE);

    if (myComputePipelineState.myGpuProgram != aProgram)
    {
      myComputePipelineState.myGpuProgram = aProgram;
      myComputePipelineState.myIsDirty = true;
    }
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetClipRect(const glm::uvec4& aClipRect)
  {
    if (myClipRect == aClipRect)
      return;

    myClipRect = aClipRect;
    myClipRectDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandContext::Reset(uint64 aFenceVal)
  {
    for (GpuRingBuffer* buf : myUploadRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myUploadRingBuffers.clear();

    for (GpuRingBuffer* buf : myConstantRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myConstantRingBuffers.clear();

    myGraphicsPipelineState = GraphicsPipelineState();
    myComputePipelineState = ComputePipelineState();
    
    myViewportParams = glm::uvec4(0, 0, 1, 1);
    myClipRect = glm::uvec4(0, 0, 1, 1);
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetViewport(const glm::uvec4& uViewportParams)
  {
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetBlendState(const SharedPtr<BlendState>& aBlendState)
  {
    const SharedPtr<BlendState>& stateToSet =
      aBlendState ? aBlendState : RenderCore::GetDefaultBlendState();

    GraphicsPipelineState& pipelineState = myGraphicsPipelineState;
    const uint64 requestedHash = stateToSet->GetHash();

    if (pipelineState.myBlendState->GetHash() == requestedHash)
      return;

    pipelineState.myBlendState = stateToSet;
    pipelineState.myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState)
  {
    const SharedPtr<DepthStencilState>& stateToSet =
      aDepthStencilState ? aDepthStencilState : RenderCore::GetDefaultDepthStencilState();

    GraphicsPipelineState& pipelineState = myGraphicsPipelineState;
    uint64 requestedHash = stateToSet->GetHash();

    if (pipelineState.myDepthStencilState->GetHash() == requestedHash)
      return;

    pipelineState.myDepthStencilState = stateToSet;
    pipelineState.myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetFillMode(const FillMode eFillMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void CommandContext::SetCullMode(const CullMode eCullMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void CommandContext::SetWindingOrder(const WindingOrder eWindingOrder)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetTopologyType(TopologyType aType)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= aType != state.myTopologyType;
    state.myTopologyType = aType;
  }
//---------------------------------------------------------------------------//
  void CommandContext::SetDepthStencilRenderTarget(TextureView* aTextureView)
  {
    if (myDepthStencilTarget == aTextureView)
      return;

    ASSERT(!aTextureView || aTextureView->GetProperties().myIsRenderTarget);

    myDepthStencilTarget = aTextureView;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myDSVformat = aTextureView != nullptr ? aTextureView->GetProperties().myFormat : DataFormat::NONE;
    myGraphicsPipelineState.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void CommandContext::SetRenderTarget(TextureView* aTextureView, const uint8 aRenderTargetIndex)
  {
    ASSERT(aRenderTargetIndex < ARRAY_LENGTH(myRenderTargets));

    if (myRenderTargets[aRenderTargetIndex] == aTextureView)
      return;

    myRenderTargets[aRenderTargetIndex] = aTextureView;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myRTVformats[aRenderTargetIndex] = aTextureView != nullptr ? aTextureView->GetProperties().myFormat : DataFormat::NONE;
    myGraphicsPipelineState.myIsDirty = true;

    uint numRenderTargets = 0u;
    for (const TextureView* rt : myRenderTargets)
      if (rt != nullptr)
        ++numRenderTargets;

    myGraphicsPipelineState.myNumRenderTargets = numRenderTargets;
  }
//---------------------------------------------------------------------------//
  void CommandContext::RemoveAllRenderTargets()
  {
    memset(myRenderTargets, 0, sizeof(myRenderTargets));
    myDepthStencilTarget = nullptr;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myNumRenderTargets = 0u;
    for (uint i = 0u; i < ARRAY_LENGTH(myGraphicsPipelineState.myRTVformats); ++i)
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;

    myGraphicsPipelineState.myDSVformat = DataFormat::NONE;
    myGraphicsPipelineState.myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandContext::UpdateBufferData(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize)
  {
    ASSERT(aDestOffset + aByteSize <= aDestBuffer->GetByteSize());

    const GpuBufferProperties& bufParams = aDestBuffer->GetProperties();

    if (bufParams.myAccessType == (uint)GpuMemoryAccessType::CPU_WRITE)
    {
      uint8* dest = static_cast<uint8*>(aDestBuffer->Lock(GpuResoruceLockOption::WRITE));
      ASSERT(dest != nullptr);
      memcpy(dest + aDestOffset, aDataPtr, aByteSize);
      aDestBuffer->Unlock();
    }
    else
    {
      uint64 srcOffset = 0u;
      const GpuBuffer* uploadBuffer = GetBuffer(srcOffset, GpuBufferUsage::STAGING_UPLOAD, aDataPtr, aByteSize);
      CopyBufferRegion(aDestBuffer, aDestOffset, uploadBuffer, srcOffset, aByteSize);
    }
  }
//---------------------------------------------------------------------------//
  void CommandContext::UpdateTextureData(const Texture* aDestTexture, const TextureSubLocation& aStartSubresource, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions /*= nullptr*/) // TODO: Support regions
  {
    DynamicArray<TextureSubLayout> subresourceLayouts;
    DynamicArray<uint64> subresourceOffsets;
    uint64 totalSize;
    aDestTexture->GetSubresourceLayout(aStartSubresource, aNumDatas, subresourceLayouts, subresourceOffsets, totalSize);

    uint64 uploadBufferOffset;
    const GpuBuffer* uploadBuffer = GetBuffer(uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, nullptr, totalSize);
    ASSERT(uploadBuffer);

    uint8* uploadBufferData = (uint8*) uploadBuffer->Lock(GpuResoruceLockOption::WRITE) + uploadBufferOffset;
        
    for (int i = 0; i < subresourceLayouts.size(); ++i)
    {
      const TextureSubLayout& dstLayout = subresourceLayouts[i];
      const TextureSubData& srcData = someDatas[i];

      const uint64 alignedSliceSize = dstLayout.myAlignedRowSize * dstLayout.myNumRows;

      uint8* dstSubresourceData = uploadBufferData + subresourceOffsets[i];
      uint8* srcSubresourceData = srcData.myData;
      for (int iSlice = 0; iSlice < dstLayout.myDepth; ++iSlice)
      {
        uint8* dstSliceData = dstSubresourceData + iSlice * alignedSliceSize;
        uint8* srcSliceData = srcSubresourceData + iSlice * srcData.mySliceSizeBytes;
        
        for (int iRow = 0; iRow < dstLayout.myNumRows; ++iRow)
        {
          uint8* dstRowData = dstSliceData + iRow * dstLayout.myAlignedRowSize;
          uint8* srcRowData = srcSliceData + iRow * srcData.myRowSizeBytes;

          ASSERT(dstLayout.myRowSize == srcData.myRowSizeBytes);
          memcpy(dstRowData, srcRowData, srcData.myRowSizeBytes);
        }
      }
    }
    uploadBuffer->Unlock();

    const uint startDestSubresourceIndex = aDestTexture->GetSubresourceIndex(aStartSubresource);
    for (int i = 0; i < subresourceLayouts.size(); ++i)
    {
      const TextureSubLocation dstLocation = aDestTexture->GetSubresourceLocation(startDestSubresourceIndex + i);
      const TextureRegion dstRegion{ glm::uvec3(0), glm::uvec3(UINT_MAX) };
      CopyTextureRegion(aDestTexture, dstLocation, dstRegion, uploadBuffer, uploadBufferOffset + subresourceOffsets[i]);
    }
  }
//---------------------------------------------------------------------------//
} 
