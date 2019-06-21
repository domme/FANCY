#include "fancy_core_precompile.h"
#include "CommandList.h"

#include "RenderCore.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "GpuProgramPipeline.h"
#include "Texture.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "TimeManager.h"
#include "GpuResourceView.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GraphicsPipelineState::GraphicsPipelineState()
    : myFillMode(FillMode::SOLID)
    , myCullMode(CullMode::BACK)
    , myWindingOrder(WindingOrder::CCW)
    , myNumRenderTargets(0u)
    , myDSVformat(DataFormat::UNKNOWN)
    , myTopologyType(TopologyType::TRIANGLE_LIST)
    , myIsDirty(true)
  {
    for(DataFormat& rtvFormat : myRTVformats)
      rtvFormat = DataFormat::UNKNOWN;

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

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  ComputePipelineState::ComputePipelineState()
    : myGpuProgram(nullptr)
    , myIsDirty(true)
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
  namespace Priv_CommandList
  {
    uint GetNumAllocatedQueriesPerRange(GpuQueryType aType)
    {
      switch(aType) 
      { 
        case GpuQueryType::TIMESTAMP: return 256u;
        case GpuQueryType::OCCLUSION: return 64u;
        case GpuQueryType::NUM: 
        default: ASSERT(false) return 0u;
      }
    }
  }
//---------------------------------------------------------------------------//
  CommandList::CommandList(CommandListType aType, uint someFlags)
    : myCommandListType(aType)
    , myCurrentContext(aType)
    , myIsTrackingResourceStates((someFlags & (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING) == 0)
    , myViewportParams(0, 0, 1, 1)
    , myClipRect(0, 0, 1, 1)
    , myViewportDirty(true)
    , myClipRectDirty(true)
    , myTopologyDirty(true)
    , myRenderTargetsDirty(true)
    , myShaderHasUnorderedWrites(false)
    , myRenderTargets{ nullptr }
    , myDepthStencilTarget(nullptr)
    , myNumTrackedResources(0u)
  {
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandList::AllocateQuery(GpuQueryType aType)
  {
    const uint type = (uint)aType;

    GpuQueryRange& range = myQueryRanges[type];
    if (range.myNumQueries > 0u && range.myNumUsedQueries == range.myNumQueries)
      RenderCore::FreeQueryRange(aType, range.myFirstQueryIdx, range.myNumQueries, range.myNumUsedQueries);

    if (range.myNumUsedQueries == range.myNumQueries)
    {
      range.myNumQueries = Priv_CommandList::GetNumAllocatedQueriesPerRange(aType);
      range.myFirstQueryIdx = RenderCore::AllocateQueryRange(aType, range.myNumQueries);
      range.myNumUsedQueries = 0u;
    }

    const uint queryIndex = range.myFirstQueryIdx + range.myNumUsedQueries;
    ++range.myNumUsedQueries;

    return GpuQuery(aType, queryIndex, Time::ourFrameIdx, myCommandListType);
  }
//---------------------------------------------------------------------------//
  int CommandList::FindResourceHazardEntryIdx(const GpuResource* aResource)
  {
    for (uint i = 0; i < myNumTrackedResources; ++i)
      if (myTrackedResources[i] == aResource)
        return i;

    return -1;
  }
//---------------------------------------------------------------------------//
  const GpuBuffer* CommandList::GetBuffer(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize)
  {
    DynamicArray<GpuRingBuffer*>* ringBufferList = nullptr;
    uint64 sizeStep = 2 * SIZE_MB;
    String name = "RingBuffer_";

    switch(aType) 
    { 
      case GpuBufferUsage::STAGING_UPLOAD: 
      {
        name += "STAGING_UPLOAD";
        ringBufferList = &myUploadRingBuffers;
      } break;
      case GpuBufferUsage::CONSTANT_BUFFER:
      {
        name += "CONSTANT_BUFFER";
        ringBufferList = &myConstantRingBuffers;
      } break;
      case GpuBufferUsage::VERTEX_BUFFER:
      {
        name += "VERTEX_BUFFER";
        ringBufferList = &myVertexRingBuffers;
        sizeStep = 1 * SIZE_MB;
      }
      break;
      case GpuBufferUsage::INDEX_BUFFER:
      {
        name += "INDEX_BUFFER";
        ringBufferList = &myIndexRingBuffers;
        sizeStep = 1 * SIZE_MB;
      }
      break;
      default:
      {
        ASSERT(false, "Buffertype not implemented as a ringBuffer");
        return nullptr;
      }
    }

    if (ringBufferList->empty() || ringBufferList->back()->GetFreeDataSize() < aDataSize)
      ringBufferList->push_back(RenderCore::AllocateRingBuffer(aType, MathUtil::Align(aDataSize, sizeStep), name.c_str()));

    GpuRingBuffer* ringBuffer = ringBufferList->back();
    uint64 offset = 0; 
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
  void CommandList::BindVertexBuffer(void* someData, uint64 aDataSize, uint aVertexSize)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::VERTEX_BUFFER, someData, aDataSize);

    BindVertexBuffer(buffer, aVertexSize, offset, aDataSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindIndexBuffer(void* someData, uint64 aDataSize, uint anIndexSize)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::INDEX_BUFFER, someData, aDataSize);

    BindIndexBuffer(buffer, anIndexSize, offset, aDataSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindConstantBuffer(void* someData, uint64 aDataSize, uint aRegisterIndex)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::CONSTANT_BUFFER, someData, aDataSize);
    
    GpuBufferViewProperties viewProperties;
    viewProperties.mySize = aDataSize;
    viewProperties.myIsConstantBuffer = true;
    viewProperties.myOffset = offset;

    BindBuffer(buffer, viewProperties, aRegisterIndex);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Render Context:
//---------------------------------------------------------------------------//
  void CommandList::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    ASSERT(myCommandListType == CommandListType::Graphics);
    
    myCurrentContext = CommandListType::Graphics;
    if (myGraphicsPipelineState.myGpuProgramPipeline != aGpuProgramPipeline)
    {
      myGraphicsPipelineState.myGpuProgramPipeline = aGpuProgramPipeline;
      myGraphicsPipelineState.myIsDirty = true;

      bool hasUnorderedWrites = false;
      for (const SharedPtr<GpuProgram>& gpuProgram : aGpuProgramPipeline->myGpuPrograms)
        if(gpuProgram != nullptr)
          hasUnorderedWrites |= gpuProgram->myProperties.myHasUnorderedWrites;

      myShaderHasUnorderedWrites = hasUnorderedWrites;
    }
  }
//---------------------------------------------------------------------------//
  void CommandList::SetComputeProgram(const GpuProgram* aProgram)
  {
    ASSERT(aProgram->myProperties.myShaderStage == ShaderStage::COMPUTE);
    ASSERT(myCommandListType == CommandListType::Graphics || myCommandListType == CommandListType::Compute);

    myCurrentContext = CommandListType::Compute;
    if (myComputePipelineState.myGpuProgram != aProgram)
    {
      myComputePipelineState.myGpuProgram = aProgram;
      myComputePipelineState.myIsDirty = true;
      myShaderHasUnorderedWrites = aProgram->myProperties.myHasUnorderedWrites;
    }
  }
//---------------------------------------------------------------------------//
  void CommandList::SetClipRect(const glm::uvec4& aClipRect)
  {
    if (myClipRect == aClipRect)
      return;

    myClipRect = aClipRect;
    myClipRectDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandList::ReleaseGpuResources(uint64 aFenceVal)
  {
    for (GpuRingBuffer* buf : myUploadRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myUploadRingBuffers.clear();

    for (GpuRingBuffer* buf : myConstantRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myConstantRingBuffers.clear();

    for (GpuRingBuffer* buf : myVertexRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myVertexRingBuffers.clear();

    for (GpuRingBuffer* buf : myIndexRingBuffers)
      RenderCore::ReleaseRingBuffer(buf, aFenceVal);
    myIndexRingBuffers.clear();

    for (uint i = 0u; i < (uint)GpuQueryType::NUM; ++i)
    {
      GpuQueryRange& range = myQueryRanges[i];
      if (range.myNumUsedQueries > 0)
        RenderCore::FreeQueryRange((GpuQueryType)i, range.myFirstQueryIdx, range.myNumQueries, range.myNumUsedQueries);
      range = { 0u, 0u, 0u };
    }
  }
//---------------------------------------------------------------------------//
  void CommandList::Reset(uint someFlags)
  {
    ASSERT(!IsOpen(), "Reset() called on open command list. Gpu-resources will not get freed! Did you forget to execute the command list?");

    myGraphicsPipelineState = GraphicsPipelineState();
    myComputePipelineState = ComputePipelineState();
    myNumTrackedResources = 0u;
    myIsTrackingResourceStates = (someFlags & (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING) == 0;
    
    myViewportParams = glm::uvec4(0, 0, 1, 1);
    myClipRect = glm::uvec4(0, 0, 1, 1);
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));
  }
//---------------------------------------------------------------------------//
  void CommandList::SetViewport(const glm::uvec4& uViewportParams)
  {
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandList::SetBlendState(const SharedPtr<BlendState>& aBlendState)
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
  void CommandList::SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState)
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
  void CommandList::SetFillMode(const FillMode eFillMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void CommandList::SetCullMode(const CullMode eCullMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void CommandList::SetWindingOrder(const WindingOrder eWindingOrder)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
  }
//---------------------------------------------------------------------------//
  void CommandList::SetTopologyType(TopologyType aType)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;

    const bool dirty = aType != state.myTopologyType;
    myTopologyDirty |= dirty;
    state.myIsDirty |= dirty;
    state.myTopologyType = aType;
  }
//---------------------------------------------------------------------------//
  void CommandList::SetRenderTarget(TextureView* aColorTarget, TextureView* aDepthStencil)
  {
    const uint newNumRenderTargets = aColorTarget == nullptr ? 0 : 1;
    const DataFormat colorFormat = aColorTarget != nullptr ? aColorTarget->GetProperties().myFormat : DataFormat::NONE;
    const DataFormat dsvFormat = aDepthStencil != nullptr ? aDepthStencil->GetProperties().myFormat : DataFormat::NONE;

    bool pipelineStateDirty =
      myGraphicsPipelineState.myNumRenderTargets != newNumRenderTargets || myGraphicsPipelineState.myRTVformats[0] == colorFormat ||
      myGraphicsPipelineState.myDSVformat != dsvFormat;

    const bool renderTargetsDirty = myRenderTargets[0] != aColorTarget || myDepthStencilTarget != aDepthStencil;
                  
    if (!pipelineStateDirty && !renderTargetsDirty)
      return;

    myRenderTargets[0] = aColorTarget;
    for (uint i = 1; i < ARRAY_LENGTH(myRenderTargets); ++i)
      myRenderTargets[i] = nullptr;

    myDepthStencilTarget = aDepthStencil;

    pipelineStateDirty |= myGraphicsPipelineState.myNumRenderTargets != newNumRenderTargets;
    myGraphicsPipelineState.myNumRenderTargets = newNumRenderTargets;

    pipelineStateDirty |= myGraphicsPipelineState.myRTVformats[0] != colorFormat;
    myGraphicsPipelineState.myRTVformats[0] = colorFormat;
    
    for (uint i = 1; i < ARRAY_LENGTH(myRenderTargets); ++i)
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;

    pipelineStateDirty |= myGraphicsPipelineState.myDSVformat != dsvFormat;
    myGraphicsPipelineState.myDSVformat = dsvFormat;
    
    myGraphicsPipelineState.myIsDirty |= pipelineStateDirty;
    myRenderTargetsDirty |= renderTargetsDirty;
  }
//---------------------------------------------------------------------------//
  void CommandList::SetRenderTargets(TextureView** someColorTargets, uint aNumColorTargets, TextureView* aDepthStencil)
  {
    ASSERT(aNumColorTargets <= ARRAY_LENGTH(myRenderTargets));

    bool renderTargetsDirty = false;
    bool pipelineStateDirty = aNumColorTargets != myGraphicsPipelineState.myNumRenderTargets;

    for (uint i = 0u; i < aNumColorTargets; ++i)
    {
      TextureView* colorTarget = someColorTargets[i];
      ASSERT(colorTarget != nullptr);

      const DataFormat colorFormat = colorTarget->GetProperties().myFormat;

      pipelineStateDirty |= myGraphicsPipelineState.myRTVformats[i] != colorFormat;
      myGraphicsPipelineState.myRTVformats[i] = colorFormat;

      renderTargetsDirty |= myRenderTargets[i] != colorTarget;
      myRenderTargets[i] = colorTarget;
    }

    for (uint i = aNumColorTargets; i < ARRAY_LENGTH(myRenderTargets); ++i)
    {
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;
      myRenderTargets[i] = nullptr;
    }

    renderTargetsDirty |= myDepthStencilTarget != aDepthStencil;
    myDepthStencilTarget = aDepthStencil;

    const DataFormat dsvFormat = aDepthStencil != nullptr ? aDepthStencil->GetProperties().myFormat : DataFormat::NONE;
    pipelineStateDirty |= myGraphicsPipelineState.myDSVformat != dsvFormat;
    myGraphicsPipelineState.myDSVformat = dsvFormat;

    myGraphicsPipelineState.myIsDirty |= pipelineStateDirty;
    myRenderTargetsDirty |= renderTargetsDirty;
  }
//---------------------------------------------------------------------------//
  void CommandList::RemoveAllRenderTargets()
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
  void CommandList::UpdateBufferData(const GpuBuffer* aDestBuffer, uint64 aDestOffset, const void* aDataPtr, uint64 aByteSize)
  {
    ASSERT(aDestOffset + aByteSize <= aDestBuffer->GetByteSize());
    
    uint64 srcOffset = 0u;
    const GpuBuffer* uploadBuffer = GetBuffer(srcOffset, GpuBufferUsage::STAGING_UPLOAD, aDataPtr, aByteSize);
    CopyBufferRegion(aDestBuffer, aDestOffset, uploadBuffer, srcOffset, aByteSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::UpdateTextureData(const Texture* aDestTexture, const TextureSubLocation& aStartSubresource, const TextureSubData* someDatas, uint aNumDatas /*, const TextureRegion* someRegions /*= nullptr*/) // TODO: Support regions
  {
    DynamicArray<TextureSubLayout> subresourceLayouts;
    DynamicArray<uint64> subresourceOffsets;
    uint64 totalSize;
    aDestTexture->GetSubresourceLayout(aStartSubresource, aNumDatas, subresourceLayouts, subresourceOffsets, totalSize);

    uint64 uploadBufferOffset;
    const GpuBuffer* uploadBuffer = GetBuffer(uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, nullptr, totalSize);
    ASSERT(uploadBuffer);

    uint8* uploadBufferData = (uint8*) uploadBuffer->Map(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize);

    const uint numSubresources = glm::min(aNumDatas, (uint) subresourceLayouts.size());
    for (uint i = 0; i < numSubresources; ++i)
    {
      const TextureSubLayout& dstLayout = subresourceLayouts[i];
      const TextureSubData& srcData = someDatas[i];

      const uint64 alignedSliceSize = dstLayout.myAlignedRowSize * dstLayout.myNumRows;

      uint8* dstSubresourceData = uploadBufferData + subresourceOffsets[i];
      uint8* srcSubresourceData = srcData.myData;
      for (uint iSlice = 0; iSlice < dstLayout.myDepth; ++iSlice)
      {
        uint8* dstSliceData = dstSubresourceData + iSlice * alignedSliceSize;
        uint8* srcSliceData = srcSubresourceData + iSlice * srcData.mySliceSizeBytes;
        
        for (uint iRow = 0; iRow < dstLayout.myNumRows; ++iRow)
        {
          uint8* dstRowData = dstSliceData + iRow * dstLayout.myAlignedRowSize;
          uint8* srcRowData = srcSliceData + iRow * srcData.myRowSizeBytes;

          ASSERT(dstLayout.myRowSize == srcData.myRowSizeBytes);
          memcpy(dstRowData, srcRowData, srcData.myRowSizeBytes);
        }
      }
    }
    uploadBuffer->Unmap(GpuResourceMapMode::WRITE_UNSYNCHRONIZED, uploadBufferOffset, totalSize);

    const uint startDestSubresourceIndex = aDestTexture->GetSubresourceIndex(aStartSubresource);
    for (uint i = 0; i < numSubresources; ++i)
    {
      const TextureSubLocation dstLocation = aDestTexture->GetSubresourceLocation(startDestSubresourceIndex + i);
      CopyTextureRegion(aDestTexture, dstLocation, glm::uvec3(0u), uploadBuffer, uploadBufferOffset + subresourceOffsets[i]);
    }
  }
//---------------------------------------------------------------------------//
  void CommandList::SubresourceBarrier(const GpuResource** someResources, const uint16** someSubResourceLists,
    const uint* someNumSubresources, const GpuResourceUsageState* someSrcStates,
    const GpuResourceUsageState* someDstStates, uint aNumResources, CommandListType aSrcQueue, CommandListType aDstQueue)
  {
    if (myIsTrackingResourceStates)
    {
      for (uint iRes = 0u; iRes < aNumResources; ++iRes)
      {
        const GpuResource* resource = someResources[iRes];
        const GpuResourceUsageState srcState = someSrcStates[iRes];
        const GpuResourceUsageState dstState = someDstStates[iRes];
        ASSERT(srcState != dstState);
        ASSERT(dstState != GpuResourceUsageState::UNKNOWN);

        // Find the local hazard-state for this resource if it already has one. If not, the srcState must be UNKNOWN
        const int resourceHazardIdx = FindResourceHazardEntryIdx(resource);
        ResourceStateTracking* trackingData = nullptr;
        if (resourceHazardIdx < 0)
        {
          ASSERT(myNumTrackedResources < ARRAY_LENGTH(myTrackedResources));
          myTrackedResources[myNumTrackedResources] = resource;
          trackingData = &myResourceStateTrackings[myNumTrackedResources];
          ++myNumTrackedResources;

          trackingData->mySubresources.resize(resource->myNumSubresources);
          for (uint i = 0; i < resource->myNumSubresources; ++i)
          {
            // UNKNOWN in the local hazard entry means that a subresource hasn't been touched by this command list
            trackingData->mySubresources[i] = { GpuResourceUsageState::UNKNOWN, GpuResourceUsageState::UNKNOWN, GpuResourceUsageState::UNKNOWN };
          }
        }
        else
        {
          trackingData = &myResourceStateTrackings[resourceHazardIdx];
        }

        // Transition all subresources?
        if (someSubResourceLists == nullptr || someNumSubresources == nullptr || someNumSubresources[iRes] >= resource->myNumSubresources)
        {
          for (uint i = 0; i < resource->myNumSubresources; ++i)  // TODO: Optimize with a flag deciding if all subresources are in the same state
          {
            SubresourceStateTracking& subTrackingData = trackingData->mySubresources[i];

            ASSERT(entry->mySubresourceStates[i] == srcState);
            trackingData->mySubresourceStates[i] = dstState;
            if (trackingData->myFirstSubresourceStates[i] == GpuResourceUsageState::UNKNOWN)
              trackingData->myFirstSubresourceStates[i] = dstState;
          }
        }
        else
        {
          const uint16* subresourceList = someSubResourceLists[iRes];
          const uint numSubresources = someNumSubresources[iRes];
          for (uint i = 0; i < numSubresources; ++i)
          {
            const uint16 subIdx = subresourceList[i];
            ASSERT(subIdx < resource->myNumSubresources);
            ASSERT(entry->mySubresourceStates[subIdx] == srcState);
            trackingData->mySubresourceStates[subIdx] = dstState;
            if (trackingData->myFirstSubresourceStates[subIdx] == GpuResourceUsageState::UNKNOWN)
              trackingData->myFirstSubresourceStates[subIdx] = dstState;
          }
        }
      }
    }

    // Queue the actual barrier
    SubresourceBarrierInternal(someResources, someSubResourceLists, someNumSubresources, someSrcStates, someDstStates, aNumResources, aSrcQueue, aDstQueue);
  }
//---------------------------------------------------------------------------//
  void CommandList::SubresourceBarrier(const GpuResource* aResource, const uint16* aSubresourceList, uint aNumSubresources,
    GpuResourceUsageState aSrcState, GpuResourceUsageState aDstState, CommandListType aSrcQueue /*=CommandListType::UNKNOWN*/, CommandListType aDstQueue /*=CommandListType::UNKNOWN*/)
  {
    const CommandListType srcQueue = aSrcQueue != CommandListType::UNKNOWN ? aSrcQueue : myCommandListType;
    const CommandListType dstQueue = aDstQueue != CommandListType::UNKNOWN ? aDstQueue : myCommandListType;
    SubresourceBarrier(&aResource, &aSubresourceList, &aNumSubresources, &aSrcState, &aDstState, 1u, srcQueue, dstQueue);
  }
//---------------------------------------------------------------------------//
  void CommandList::SubresourceBarrier(const GpuResourceView* aResourceView, GpuResourceUsageState aSrcState, 
    GpuResourceUsageState aDstState, CommandListType aSrcQueue /*=CommandListType::UNKNOWN*/, CommandListType aDstQueue /*=CommandListType::UNKNOWN*/)
  {
    const CommandListType srcQueue = aSrcQueue != CommandListType::UNKNOWN ? aSrcQueue : myCommandListType;
    const CommandListType dstQueue = aDstQueue != CommandListType::UNKNOWN ? aDstQueue : myCommandListType;

    if (aResourceView->mySubresources[1].empty())  // Single-plane resource
    {
      const uint16* subresourceList = aResourceView->mySubresources[0].data();
      const uint numSubresources = (uint)aResourceView->mySubresources->size();
      ASSERT(numSubresources > 0);

      SubresourceBarrier(aResourceView->myResource.get(), subresourceList, numSubresources, aSrcState, aDstState, srcQueue, dstQueue);
    }
    else
    {
      const GpuResource* resources[] = { aResourceView->myResource.get(), aResourceView->myResource.get() };
      const uint16* subresourceLists[] = { aResourceView->mySubresources[0].data(), aResourceView->mySubresources[1].data() };
      const uint numSubresources[] = { (uint)aResourceView->mySubresources[0].size(), (uint)aResourceView->mySubresources[1].size() };
      const GpuResourceUsageState srcStates[] = { aSrcState, aSrcState };
      const GpuResourceUsageState dstStates[] = { aDstState, aDstState };
      ASSERT(numSubresources[0] > 0);
      ASSERT(numSubresources[1] > 0);

      SubresourceBarrier(resources, subresourceLists, numSubresources, srcStates, dstStates, 2u, srcQueue, dstQueue);
    }
  }
//---------------------------------------------------------------------------//
  void CommandList::ResourceBarrier(const GpuResource* aResource, GpuResourceUsageState aSrcState, GpuResourceUsageState aDstState, CommandListType aSrcQueue /*=CommandListType::UNKNOWN*/, CommandListType aDstQueue /*=CommandListType::UNKNOWN*/)
  {
    const CommandListType srcQueue = aSrcQueue != CommandListType::UNKNOWN ? aSrcQueue : myCommandListType;
    const CommandListType dstQueue = aDstQueue != CommandListType::UNKNOWN ? aDstQueue : myCommandListType;
    SubresourceBarrier(&aResource, nullptr, nullptr, &aSrcState, &aDstState, 1u, srcQueue, dstQueue);
  }
//---------------------------------------------------------------------------//
} 
