#include "CommandContext.h"
#include "RenderCore.h"
#include "MathUtil.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "GpuProgramPipeline.h"
#include "Texture.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
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
  uint GraphicsPipelineState::GetHash() const
  {
    uint hash = 0u;
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
      MathUtil::hash_combine(hash, reinterpret_cast<uint>(myRTVformats));

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
  uint ComputePipelineState::GetHash() const
  {
    uint hash = 0u;
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
    ASSERT(aProgram->getShaderStage() == ShaderStage::COMPUTE);

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
    const uint requestedHash = stateToSet->GetHash();

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
    uint requestedHash = stateToSet->GetHash();

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
  void CommandContext::SetDepthStencilRenderTarget(Texture* pDStexture)
  {
    if (myDepthStencilTarget == pDStexture)
      return;

    myDepthStencilTarget = pDStexture;
    myRenderTargetsDirty = true;

    ASSERT(pDStexture == nullptr || pDStexture->GetParameters().bIsDepthStencil);
    myGraphicsPipelineState.myDSVformat = pDStexture != nullptr ? pDStexture->GetParameters().eFormat : DataFormat::NONE;
    myGraphicsPipelineState.myIsDirty = true;
  }
  //---------------------------------------------------------------------------//
  void CommandContext::SetRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
  {
    if (myRenderTargets[u8RenderTargetIndex] == pRTTexture)
      return;

    myRenderTargets[u8RenderTargetIndex] = pRTTexture;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myRTVformats[u8RenderTargetIndex] = pRTTexture != nullptr ? pRTTexture->GetParameters().eFormat : DataFormat::NONE;
    myGraphicsPipelineState.myIsDirty = true;

    uint numRenderTargets = 0u;
    for (const Texture* rt : myRenderTargets)
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
    for (uint32 i = 0u; i < ARRAY_LENGTH(myGraphicsPipelineState.myRTVformats); ++i)
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;

    myGraphicsPipelineState.myDSVformat = DataFormat::NONE;
    // myGraphicsPipelineState.myIsDirty = true;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Compute Context
//---------------------------------------------------------------------------//








} }
