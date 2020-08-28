#include "fancy_core_precompile.h"
#include "CommandList.h"

#include "RenderCore.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "ShaderPipeline.h"
#include "Texture.h"
#include "Shader.h"
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
    , myVertexInputLayout(nullptr)
  {
    for(DataFormat& rtvFormat : myRTVformats)
      rtvFormat = DataFormat::UNKNOWN;

    myDepthStencilState = RenderCore::GetDefaultDepthStencilState().get();
    myBlendState = RenderCore::GetDefaultBlendState().get();
  }
//---------------------------------------------------------------------------//
  uint64 GraphicsPipelineState::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, static_cast<uint>(myFillMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myCullMode));
    MathUtil::hash_combine(hash, static_cast<uint>(myWindingOrder));
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myDepthStencilState));
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myBlendState));
    MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myVertexInputLayout));
    MathUtil::hash_combine(hash, myShaderPipeline != nullptr ? myShaderPipeline->GetHash() : 0u);  // TODO: This should not be needed, the shaderbytecode hash should be enough

    if (myShaderPipeline != nullptr)
      MathUtil::hash_combine(hash, myShaderPipeline->GetShaderByteCodeHash());

    MathUtil::hash_combine(hash, myNumRenderTargets);

    for (uint i = 0u; i < RenderConstants::kMaxNumRenderTargets; ++i)
      MathUtil::hash_combine(hash, reinterpret_cast<uint64>(myRTVformats));

    MathUtil::hash_combine(hash, static_cast<uint>(myDSVformat));

    return hash;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  ComputePipelineState::ComputePipelineState()
    : myShaderPipeline(nullptr)
    , myIsDirty(true)
  {
  }
//---------------------------------------------------------------------------//
  uint64 ComputePipelineState::GetHash() const
  {
    uint64 hash = 0u;
    MathUtil::hash_combine(hash, myShaderPipeline != nullptr ? myShaderPipeline->GetHash() : 0u);  // TODO: This should not be needed, the shaderbytecode hash should be enough

    if (myShaderPipeline != nullptr)
      MathUtil::hash_combine(hash, myShaderPipeline->GetShaderByteCodeHash());

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
  CommandList::CommandList(CommandListType aType)
    : myCommandListType(aType)
    , myCurrentContext(aType)
    , myViewportParams(0, 0, 1, 1)
    , myClipRect(0, 0, 1, 1)
    , myIsOpen(true)
    , myViewportDirty(true)
    , myClipRectDirty(true)
    , myTopologyDirty(true)
    , myRenderTargetsDirty(true)
    , myShaderPipelineHasUnorderedWrites(false)
    , myRenderTargets{ nullptr }
    , myDepthStencilTarget(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  void CommandList::CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource)
  {
    const TextureProperties& srcProps = aSrcTexture->GetProperties();
    uint srcWidth, srcHeight, srcDepth;
    srcProps.GetSize(aSrcSubresource.myMipLevel, srcWidth, srcHeight, srcDepth);

    CopyTextureToBuffer(aDstBuffer, aDstOffset, aSrcTexture, aSrcSubresource, TextureRegion(glm::uvec3(0), glm::uvec3(srcWidth, srcHeight, srcDepth)));
  }
//---------------------------------------------------------------------------//
  void CommandList::CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource)
  {
    const TextureProperties& srcProps = aSrcTexture->GetProperties();
    uint srcWidth, srcHeight, srcDepth;
    srcProps.GetSize(aSrcSubresource.myMipLevel, srcWidth, srcHeight, srcDepth);

    const TextureProperties& dstProps = aDstTexture->GetProperties();
    uint dstWidth, dstHeight, dstDepth;
    dstProps.GetSize(aDstSubresource.myMipLevel, dstWidth, dstHeight, dstDepth);

    CopyTexture(aDstTexture, aDstSubresource, TextureRegion(glm::uvec3(0), glm::uvec3(dstWidth, dstHeight, dstDepth)),
      aSrcTexture, aSrcSubresource, TextureRegion(glm::uvec3(0), glm::uvec3(srcWidth, srcHeight, srcDepth)));
  }
//---------------------------------------------------------------------------//
  void CommandList::CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
    const TextureProperties& dstProps = aDstTexture->GetProperties();
    uint dstWidth, dstHeight, dstDepth;
    dstProps.GetSize(aDstSubresource.myMipLevel, dstWidth, dstHeight, dstDepth);

    CopyBufferToTexture(aDstTexture, aDstSubresource, TextureRegion(glm::uvec3(0), glm::uvec3(dstWidth, dstHeight, dstDepth)), aSrcBuffer, aSrcOffset);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, const char* aName, uint anArrayIndex /*= 0u*/)
  {
    const uint64 nameHash = Shader::GetParameterNameHash(aName);
    BindBuffer(aBuffer, someViewProperties, nameHash, anArrayIndex);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindResourceView(const GpuResourceView* aView, const char* aName, uint anArrayIndex /*= 0u*/)
  {
    const uint64 nameHash = Shader::GetParameterNameHash(aName);
    BindResourceView(aView, nameHash, anArrayIndex);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindSampler(const TextureSampler* aSampler, const char* aName, uint anArrayIndex /*= 0u*/)
  {
    const uint64 nameHash = Shader::GetParameterNameHash(aName);
    BindSampler(aSampler, nameHash, anArrayIndex);
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
  GpuRingBuffer* CommandList::GetUploadBuffer_Internal(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize)
  {
    eastl::vector<GpuRingBuffer*>* ringBufferList = nullptr;
    uint64 sizeStep = 2 * SIZE_MB;
    eastl::string name = "RingBuffer_";

    uint bindFlags = 0u;
    switch (aType)
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
      bindFlags |= (uint)GpuBufferBindFlags::CONSTANT_BUFFER;
    } break;
    case GpuBufferUsage::VERTEX_BUFFER:
    {
      name += "VERTEX_BUFFER";
      ringBufferList = &myVertexRingBuffers;
      sizeStep = 1 * SIZE_MB;
      bindFlags |= (uint)GpuBufferBindFlags::VERTEX_BUFFER;
    }
    break;
    case GpuBufferUsage::INDEX_BUFFER:
    {
      name += "INDEX_BUFFER";
      ringBufferList = &myIndexRingBuffers;
      sizeStep = 1 * SIZE_MB;
      bindFlags |= (uint)GpuBufferBindFlags::INDEX_BUFFER;
    }
    break;
    default:
    {
      ASSERT(false, "Buffertype not implemented as a ringBuffer");
      return nullptr;
    }
    }

    if (ringBufferList->empty() || ringBufferList->back()->GetFreeDataSize() < aDataSize)
      ringBufferList->push_back(RenderCore::AllocateRingBuffer(CpuMemoryAccessType::CPU_WRITE, bindFlags, (uint)MathUtil::Align(aDataSize, sizeStep), name.c_str()));

    GpuRingBuffer* ringBuffer = ringBufferList->back();
    uint64 offset = 0;
    bool success = true;
    if (someData != nullptr)
      success = ringBuffer->AllocateAndWrite(someData, aDataSize, offset);
    else
      success = ringBuffer->Allocate(aDataSize, offset);
    ASSERT(success);

    anOffsetOut = offset;
    return ringBuffer;
  }
//---------------------------------------------------------------------------//
  const GpuBuffer* CommandList::GetBuffer(uint64& anOffsetOut, GpuBufferUsage aType, const void* someData, uint64 aDataSize)
  {
    GpuRingBuffer* ringBuffer = GetUploadBuffer_Internal(anOffsetOut, aType, someData, aDataSize);
    ASSERT(ringBuffer != nullptr);
    return ringBuffer->GetBuffer();
  }
//---------------------------------------------------------------------------//
  const GpuBuffer* CommandList::GetMappedBuffer(uint64& anOffsetOut, GpuBufferUsage aType, uint8** someDataPtrOut, uint64 aDataSize)
  {
    GpuRingBuffer* ringBuffer = GetUploadBuffer_Internal(anOffsetOut, aType, nullptr, aDataSize);
    ASSERT(ringBuffer != nullptr);
    *someDataPtrOut = ringBuffer->GetData() + anOffsetOut;
    return ringBuffer->GetBuffer();
  }
//---------------------------------------------------------------------------//
  void CommandList::BindVertexBuffer(void* someData, uint64 aDataSize)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::VERTEX_BUFFER, someData, aDataSize);

    BindVertexBuffers(&buffer, &offset, &aDataSize, 1u);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindIndexBuffer(void* someData, uint64 aDataSize, uint anIndexSize)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::INDEX_BUFFER, someData, aDataSize);

    BindIndexBuffer(buffer, anIndexSize, offset, aDataSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::BindConstantBuffer(void* someData, uint64 aDataSize, const char* aName)
  {
    uint64 offset = 0u;
    const GpuBuffer* buffer = GetBuffer(offset, GpuBufferUsage::CONSTANT_BUFFER, someData, aDataSize);
    
    GpuBufferViewProperties viewProperties;
    viewProperties.mySize = aDataSize;
    viewProperties.myIsConstantBuffer = true;
    viewProperties.myOffset = offset;

    BindBuffer(buffer, viewProperties, aName);
  }
//---------------------------------------------------------------------------//
// Render Context:
//---------------------------------------------------------------------------//
  void CommandList::SetShaderPipeline(const ShaderPipeline* aShaderPipeline)
  {
    bool hasChanged = false;
    SetShaderPipelineInternal(aShaderPipeline, hasChanged);
  }
//---------------------------------------------------------------------------//
  void CommandList::SetShaderPipelineInternal(const ShaderPipeline* aPipeline, bool& aHasPipelineChangedOut)
  {
    ASSERT(myCommandListType != CommandListType::DMA);

    aHasPipelineChangedOut = false;

    if (aPipeline->IsComputePipeline())
    {
      myCurrentContext = CommandListType::Compute;
      if (myComputePipelineState.myShaderPipeline != aPipeline)
      {
        myComputePipelineState.myShaderPipeline = aPipeline;
        myComputePipelineState.myIsDirty = true;
        aHasPipelineChangedOut = true;
      }
    }
    else
    {
      ASSERT(myCommandListType == CommandListType::Graphics);
      myCurrentContext = CommandListType::Graphics;

      if (myGraphicsPipelineState.myShaderPipeline != aPipeline)
      {
        myGraphicsPipelineState.myShaderPipeline = aPipeline;
        myGraphicsPipelineState.myIsDirty = true;
        aHasPipelineChangedOut = true;
      }
    }

    bool hasUnorderedWrites = false;
    for (uint i = 0u; i < (uint) ShaderStage::NUM; ++i)
      if (aPipeline->GetShader(i) != nullptr)
        hasUnorderedWrites |= aPipeline->GetShader(i)->myProperties.myHasUnorderedWrites;

    myShaderPipelineHasUnorderedWrites = hasUnorderedWrites;
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
  void CommandList::TransitionResource(const GpuResource* aResource, ResourceTransition aTransition, uint someUsageFlags /* = 0u */)
  {
    TransitionResource(aResource, aResource->GetSubresources(), aTransition, someUsageFlags);
  }
//---------------------------------------------------------------------------//
  void CommandList::PostExecute(uint64 aFenceVal)
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
  void CommandList::PreBegin()
  {
    ASSERT(!IsOpen(), "PreBegin() called on open command list. Gpu-resources will not get freed! Did you forget to execute the command list?");

    myGraphicsPipelineState = GraphicsPipelineState();
    myComputePipelineState = ComputePipelineState();
    
    myViewportParams = glm::uvec4(0, 0, 1, 1);
    myClipRect = glm::uvec4(0, 0, 1, 1);
    myViewportDirty = true;
    myRenderTargetsDirty = true;
    myDepthStencilTarget = nullptr;
    memset(myRenderTargets, 0u, sizeof(myRenderTargets));

    myIsOpen = true;
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

    if (pipelineState.myBlendState == stateToSet.get())
      return;

    pipelineState.myBlendState = stateToSet.get();
    pipelineState.myIsDirty = true;
  }
//---------------------------------------------------------------------------//
  void CommandList::SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState)
  {
    const SharedPtr<DepthStencilState>& stateToSet =
      aDepthStencilState ? aDepthStencilState : RenderCore::GetDefaultDepthStencilState();

    GraphicsPipelineState& pipelineState = myGraphicsPipelineState;

    if (pipelineState.myDepthStencilState == stateToSet.get())
      return;

    pipelineState.myDepthStencilState = stateToSet.get();
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
  void CommandList::SetVertexInputLayout(const VertexInputLayout* anInputLayout)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;

    if (state.myVertexInputLayout != anInputLayout)
    {
      state.myIsDirty = true;
      state.myVertexInputLayout = anInputLayout;
    }
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
    CopyBuffer(aDestBuffer, aDestOffset, uploadBuffer, srcOffset, aByteSize);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
   /*

  void CommandList::SubresourceBarrier(const GpuResource* aResource, const SubresourceLocation& aSubresourceLocation, GpuResourceState aSrcState, GpuResourceState aDstState)
  {
    SubresourceRange subresourceRange(aSubresourceLocation.myMipLevel, 1u, aSubresourceLocation.myArrayIndex, 1u, aSubresourceLocation.myPlaneIndex, 1u);
    SubresourceBarrier(aResource, subresourceRange, aSrcState, aDstState);
  }
//---------------------------------------------------------------------------//
  void CommandList::SubresourceBarrier(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, GpuResourceState aSrcState, GpuResourceState aDstState)
  {
    ASSERT(!aSubresourceRange.IsEmpty());

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
    for (SubresourceIterator subIter = aSubresourceRange.Begin(), end = aSubresourceRange.End(); subIter != end; ++subIter)
    {
      LOG_INFO("Subresource transition (untracked): Resource %s (subresource mip: %d, array: %d, plane: %d) from %s-%s to %s-%s",
        aResource->myName.c_str(), subIter->myMipLevel, subIter->myArrayIndex, subIter->myPlaneIndex, RenderCore::CommandListTypeToString(myCommandListType), RenderCore::ResourceUsageStateToString(aSrcState),
        RenderCore::CommandListTypeToString(myCommandListType), RenderCore::ResourceUsageStateToString(aDstState));
    }
#endif

    SubresourceBarrierInternal(aResource, aSubresourceRange, aSrcState, aDstState, myCommandListType, myCommandListType);
  }
//---------------------------------------------------------------------------//
  void CommandList::SubresourceBarrier(const GpuResourceView* aResourceView, GpuResourceState aSrcState, GpuResourceState aDstState)
  {
    SubresourceBarrier(aResourceView->myResource.get(), aResourceView->mySubresourceRange, aSrcState, aDstState);
  }
//---------------------------------------------------------------------------//
  void CommandList::ResourceBarrier(const GpuResource* aResource, GpuResourceState aSrcState, GpuResourceState aDstState, CommandListType aSrcQueue, CommandListType aDstQueue)
  {
    if (aSrcState == aDstState && aSrcQueue == aDstQueue)
      return;

    ASSERT(aSrcState != GpuResourceState::UNKNOWN && aDstState != GpuResourceState::UNKNOWN);
    ASSERT(aSrcQueue != CommandListType::UNKNOWN && aDstQueue != CommandListType::UNKNOWN);
    ASSERT(GpuResourceStateTracking::QueueUnderstandsPartsOfState(myCommandListType, aSrcState));
    ASSERT(GpuResourceStateTracking::QueueUnderstandsPartsOfState(myCommandListType, aDstState));
    ASSERT(GpuResourceStateTracking::QueueUnderstandsPartsOfState(aSrcQueue, aSrcState));
    ASSERT(GpuResourceStateTracking::QueueUnderstandsPartsOfState(aDstQueue, aDstState));

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
      LOG_INFO("Resource transition: Resource %s from %s-%s to %s-%s (on %s)",
        aResource->myName.c_str(), RenderCore::CommandListTypeToString(aSrcQueue), RenderCore::ResourceUsageStateToString(aSrcState),
        RenderCore::CommandListTypeToString(aDstQueue), RenderCore::ResourceUsageStateToString(aDstState), RenderCore::CommandListTypeToString(myCommandListType));
#endif  
    SubresourceBarrierInternal(aResource, aResource->mySubresources, aSrcState, aDstState, aSrcQueue, aDstQueue);
  }
//---------------------------------------------------------------------------//
  void CommandList::ResourceBarrier(const GpuResource* aResource, GpuResourceState aSrcState, GpuResourceState aDstState)
  {
    ResourceBarrier(aResource, aSrcState, aDstState, myCommandListType, myCommandListType);
  }

  */
//---------------------------------------------------------------------------//
  void CommandList::ValidateTextureCopy(const TextureProperties& aDstProps, const SubresourceLocation& aDstSubresrource, const TextureRegion& aDstRegion,
    const TextureProperties& aSrcProps, const SubresourceLocation& aSrcSubresource, const TextureRegion& aSrcRegion) const
  {
    ASSERT(aSrcRegion.mySize == aDstRegion.mySize);
    ASSERT(aSrcRegion.mySize != glm::uvec3(0) && aDstRegion.mySize != glm::uvec3(0));

    uint dstWidth, dstHeight, dstDepth;
    aDstProps.GetSize(aDstSubresrource.myMipLevel, dstWidth, dstHeight, dstDepth);

    const glm::uvec3 dstEndTexel = aDstRegion.myPos + aDstRegion.mySize;
    ASSERT(dstEndTexel.x <= dstWidth && dstEndTexel.y <= dstHeight && dstEndTexel.z <= dstDepth);

    uint srcWidth, srcHeight, srcDepth;
    aSrcProps.GetSize(aSrcSubresource.myMipLevel, srcWidth, srcHeight, srcDepth);

    const glm::uvec3 srcEndTexel = aSrcRegion.myPos + aSrcRegion.mySize;
    ASSERT(srcEndTexel.x <= srcWidth && srcEndTexel.y <= srcHeight && srcEndTexel.z <= srcDepth);
  }
//---------------------------------------------------------------------------//
  void CommandList::ValidateTextureToBufferCopy(const GpuBufferProperties& aDstBufferProps, uint64 aDstBufferOffset,
    const TextureProperties& aSrcTextureProps, const SubresourceLocation& aSrcSubresource,
    const TextureRegion& aSrcRegion) const
  {
    ASSERT(aSrcRegion.mySize != glm::uvec3(0));

    uint subWidth, subHeight, subDepth;
    aSrcTextureProps.GetSize(aSrcSubresource.myMipLevel, subWidth, subHeight, subDepth);

    const bool entireSubresource = aSrcRegion.mySize == glm::uvec3(subWidth, subHeight, subDepth);
    if (entireSubresource)
      ASSERT(MathUtil::IsAligned(aDstBufferOffset, RenderCore::GetPlatformCaps().myTextureSubresourceBufferAlignment));

    const glm::uvec3 srcEndTexel = aSrcRegion.myPos + aSrcRegion.mySize;
    ASSERT(srcEndTexel.x <= subWidth && srcEndTexel.y <= subHeight && srcEndTexel.z <= subDepth);

    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    const uint64 alignedBufferOffset = MathUtil::Align(aDstBufferOffset, caps.myTextureSubresourceBufferAlignment);

    const uint64 bufferCapacity = aDstBufferProps.myElementSizeBytes * aDstBufferProps.myElementSizeBytes;
    ASSERT(bufferCapacity > alignedBufferOffset);

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(aSrcTextureProps.myFormat);
    const uint64 alignedRowSize = MathUtil::Align(aSrcRegion.mySize.x * formatInfo.myCopyableSizePerPlane[aSrcSubresource.myPlaneIndex], RenderCore::GetPlatformCaps().myTextureRowAlignment);
    uint64 requiredBufferSize = alignedRowSize * aSrcRegion.mySize.y * aSrcRegion.mySize.z;
    if (entireSubresource)
      requiredBufferSize = MathUtil::Align(requiredBufferSize, RenderCore::GetPlatformCaps().myTextureSubresourceBufferAlignment);

    const uint64 freeBufferSize = bufferCapacity - alignedBufferOffset;
    ASSERT(freeBufferSize >= requiredBufferSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::ValidateBufferToTextureCopy(const TextureProperties& aDstTexProps,
                                                const SubresourceLocation& aDstSubresource, const TextureRegion& aDstRegion,
                                                const GpuBufferProperties& aSrcBufferProps, uint64 aSrcBufferOffset) const
  {
    ASSERT(aDstRegion.mySize != glm::uvec3(0));

    uint subWidth, subHeight, subDepth;
    aDstTexProps.GetSize(aDstSubresource.myMipLevel, subWidth, subHeight, subDepth);
    ASSERT(aDstRegion.myPos.x + aDstRegion.mySize.x <= subWidth);
    ASSERT(aDstRegion.myPos.y + aDstRegion.mySize.y <= subHeight);
    ASSERT(aDstRegion.myPos.z + aDstRegion.mySize.z <= subDepth);

    const bool entireSubresource = aDstRegion.mySize == glm::uvec3(subWidth, subHeight, subDepth);
    if (entireSubresource)
      ASSERT(MathUtil::IsAligned(aSrcBufferOffset, RenderCore::GetPlatformCaps().myTextureSubresourceBufferAlignment));
    
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(aDstTexProps.myFormat);
    const uint64 alignedRowSize = MathUtil::Align(aDstRegion.mySize.x * formatInfo.myCopyableSizePerPlane[aDstSubresource.myPlaneIndex], RenderCore::GetPlatformCaps().myTextureRowAlignment);
    uint64 requiredBufferSize = alignedRowSize * aDstRegion.mySize.y * aDstRegion.mySize.z;
    if (entireSubresource)
      requiredBufferSize = MathUtil::Align(requiredBufferSize, RenderCore::GetPlatformCaps().myTextureSubresourceBufferAlignment);

    const uint64 bufferCapacity = aSrcBufferProps.myElementSizeBytes * aSrcBufferProps.myNumElements;
    ASSERT(aSrcBufferOffset < bufferCapacity);
    ASSERT(bufferCapacity >= aSrcBufferOffset + requiredBufferSize);
  }
//---------------------------------------------------------------------------//
  void CommandList::ValidateBufferCopy(const GpuBufferProperties& aDstProps, uint64 aDstOffset, 
    const GpuBufferProperties& aSrcProps, uint64 aSrcOffset, uint64 aSize) const
  {
    ASSERT(aSize > 0u);

    const uint64 dstBufferCapacity = aDstProps.myNumElements * aDstProps.myElementSizeBytes;
    ASSERT(aDstOffset < dstBufferCapacity);
    ASSERT(aDstOffset + aSize <= dstBufferCapacity);

    const uint64 dstFreeSize = dstBufferCapacity - aDstOffset;
    ASSERT(aSize <= dstFreeSize);

    const uint64 srcBufferCapacity = aSrcProps.myNumElements * aSrcProps.myElementSizeBytes;
    ASSERT(aSrcOffset < srcBufferCapacity);
    ASSERT(aSrcOffset + aSize <= srcBufferCapacity);
  }
//---------------------------------------------------------------------------//
} 
