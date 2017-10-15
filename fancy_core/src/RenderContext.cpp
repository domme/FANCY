#include "FancyCorePrerequisites.h"
#include "RenderContext.h"
#include "RenderCore.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "MathUtil.h"
#include "GpuProgramPipeline.h"
#include "Texture.h"

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
  uint GraphicsPipelineState::GetHash()
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
  RenderContext::RenderContext()
    : CommandContext(CommandListType::Graphics)
    , myViewportParams(0, 0, 1, 1)
    , myClipRect(0, 0, 1, 1)
    , myViewportDirty(true)
    , myClipRectDirty(true)
    , myRenderTargetsDirty(true)
    , myDepthStencilTarget(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  void RenderContext::SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline)
  {
    if (myGraphicsPipelineState.myGpuProgramPipeline != aGpuProgramPipeline)
    {
      myGraphicsPipelineState.myGpuProgramPipeline = aGpuProgramPipeline;
      myGraphicsPipelineState.myIsDirty = true;
    }
  }
//---------------------------------------------------------------------------//
  void RenderContext::SetClipRect(const glm::uvec4& aClipRect)
  {
    if (myClipRect == aClipRect)
      return;

    myClipRect = aClipRect;
    myClipRectDirty = true;
  }
//---------------------------------------------------------------------------//
  void RenderContext::SetViewport(const glm::uvec4& uViewportParams)
  {
    if (myViewportParams == uViewportParams)
      return;

    myViewportParams = uViewportParams;
    myViewportDirty = true;
  }
//---------------------------------------------------------------------------//
  void RenderContext::SetBlendState(const SharedPtr<BlendState>& aBlendState)
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
  void RenderContext::SetDepthStencilState(const SharedPtr<DepthStencilState>& aDepthStencilState)
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
  void RenderContext::SetFillMode(const FillMode eFillMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eFillMode != state.myFillMode;
    state.myFillMode = eFillMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContext::SetCullMode(const CullMode eCullMode)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eCullMode != state.myCullMode;
    state.myCullMode = eCullMode;
  }
  //---------------------------------------------------------------------------//
  void RenderContext::SetWindingOrder(const WindingOrder eWindingOrder)
  {
    GraphicsPipelineState& state = myGraphicsPipelineState;
    state.myIsDirty |= eWindingOrder != state.myWindingOrder;
    state.myWindingOrder = eWindingOrder;
  }
  //---------------------------------------------------------------------------//
  void RenderContext::SetDepthStencilRenderTarget(Texture* pDStexture)
  {
    if (myDepthStencilTarget == pDStexture)
      return;

    myDepthStencilTarget = pDStexture;
    myRenderTargetsDirty = true;

    ASSERT(pDStexture == nullptr || pDStexture->GetParameters().bIsDepthStencil);
    myGraphicsPipelineState.myDSVformat = pDStexture != nullptr ? pDStexture->GetParameters().eFormat : DataFormat::NONE;
  }
  //---------------------------------------------------------------------------//
  void RenderContext::SetRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex)
  {
    if (myRenderTargets[u8RenderTargetIndex] == pRTTexture)
      return;

    myRenderTargets[u8RenderTargetIndex] = pRTTexture;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myRTVformats[u8RenderTargetIndex] = pRTTexture != nullptr ? pRTTexture->GetParameters().eFormat : DataFormat::NONE;

    uint numRenderTargets = 0u;
    for (const Texture* rt : myRenderTargets)
      if (rt != nullptr)
        ++numRenderTargets;

    myGraphicsPipelineState.myNumRenderTargets = numRenderTargets;
  }
  //---------------------------------------------------------------------------//
  void RenderContext::RemoveAllRenderTargets()
  {
    memset(myRenderTargets, 0, sizeof(myRenderTargets));
    myDepthStencilTarget = nullptr;
    myRenderTargetsDirty = true;

    myGraphicsPipelineState.myNumRenderTargets = 0u;
    for (uint32 i = 0u; i < ARRAY_LENGTH(myGraphicsPipelineState.myRTVformats); ++i)
      myGraphicsPipelineState.myRTVformats[i] = DataFormat::NONE;

    myGraphicsPipelineState.myDSVformat = DataFormat::NONE;
  }
//---------------------------------------------------------------------------//

} }

