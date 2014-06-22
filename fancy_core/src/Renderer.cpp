#include "Renderer.h"

using namespace FANCY::Core::Rendering;

Renderer::Renderer() :
  m_pImpl(0),
  m_uPipelineRebindMask(static_cast<uint>(PipelineRebindFlags::ALL)),
  m_bChangingPipelineState(false),
  m_bChangingResourceState(false),
  LoadableObject::LoadableObject()
{
  memset(m_uResourceRebindMask, static_cast<uint>(ResourceRebindFlags::ALL), sizeof(m_uResourceRebindMask));
}

Renderer::~Renderer()
{

}

bool Renderer::_init()
{
  if (!m_pImpl)
  {
    LOG("ERROR: Tried to init the Renderer without an implementation set");
    return false;
  }

  return m_pImpl->init();
}

bool Renderer::_destroy()
{
  if (!m_pImpl)
  {
    return true;
  }

  return m_pImpl->destroy();
}

bool Renderer::setImplementation( RendererImpl* pImpl )
{
  if (!pImpl || pImpl == m_pImpl)
  {
    return;
  }

  if (m_pImpl)
  {
    // The implementation changed: Unload the current one
    m_pImpl->destroy();
  }

  m_pImpl = pImpl;

  if (isInitialized()) 
  {
    // We are hot-swapping the implementation. Init it here
    m_pImpl->init();

    // Set all render-states dirty and apply them
    // TODO: Notify textures, buffers, GPUprograms to reload in the new impl!
    beginChangePipelineState();
    {
      m_uPipelineRebindMask = static_cast<uint>(PipelineRebindFlags::ALL);
    }
    endChangePipelineState();

    beginChangeResourceState();
    {
      memset(m_uResourceRebindMask, static_cast<uint>(ResourceRebindFlags::ALL), sizeof(m_uResourceRebindMask));
    }
    endChangeResourceState();
  }
}
//////////////////////////////////////////////////////////////////////////
void Renderer::beginChangePipelineState()
{
  ASSERT_M(!m_bChangingPipelineState, "Call to beginChangePipelineState before end detected!");
  m_bChangingPipelineState = true;
  m_uPipelineRebindMask = static_cast<uint>(PipelineRebindFlags::NONE);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::endChangePipelineState()
{
  ASSERT_M(!m_bChangingPipelineState, "Call to endChangePipelineState without begin detected!");
  ASSERT_M(m_pImpl, "No implementation detected");

  // Call the implementation depending on the set rebind flags
  // TODO: Might be a bit inefficient to call the impl multiple times...
  //       let the impl handle the rebindMask in the future?
  if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::ALL)) > 0)
  {
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL)) > 0)
    {
      m_pImpl->_setDepthStencilState(m_clPipelineState.clDepthStencilState);
    }
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::BLENDING)) > 0)
    {
      m_pImpl->_setBlendState(m_clPipelineState.clBlendState);
    }
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::FILLMODE)) > 0)
    {
      m_pImpl->_setFillMode(m_clPipelineState.eFillMode);
    }
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::CULLMODE)) > 0)
    {
      m_pImpl->_setCullMode(m_clPipelineState.eCullMode);
    }
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::WINDINGORDER)) > 0)
    {
      m_pImpl->_setWindingOrder(m_clPipelineState.eWindingOrder);
    }
    if ((m_uPipelineRebindMask & static_cast<uint>(PipelineRebindFlags::RENDERTARGETS)) > 0)
    {
      m_pImpl->_bindRenderTargets(m_clPipelineState.pBoundRenderTargets,
                                  FANCY_MAX_NUM_BOUND_RENDERTARGETS);
    }
  }

  m_uPipelineRebindMask = static_cast<uint>(PipelineRebindFlags::NONE);
  m_bChangingPipelineState = false;
}
//////////////////////////////////////////////////////////////////////////
void Renderer::beginChangeResourceState()
{
  ASSERT_M(!m_bChangingResourceState, "Call to beginChangingResourceState before end detected!");
  m_bChangingResourceState = true;
  memset(m_uResourceRebindMask, static_cast<uint>(ResourceRebindFlags::NONE), sizeof(m_uResourceRebindMask));
}
//////////////////////////////////////////////////////////////////////////
void Renderer::endChangeResourceState()
{
  ASSERT_M(!m_bChangingResourceState, "Call to endChangeResourceState without begin detected!");
  ASSERT_M(m_pImpl, "No implementation detected");

  // Call the implementation depending on the set rebind flags
  // TODO: Might be a bit inefficient to call the impl multiple times...
  //       let the impl handle the rebindMask in the future?
  for(uint iShaderStage = 0; iShaderStage < static_cast<uint>(ShaderStage::NUM); ++iShaderStage)
  {
    uint uRebindMask = m_uResourceRebindMask[iShaderStage];
    if((uRebindMask & static_cast<uint>(ResourceRebindFlags::ALL)) > 0)
    {
      if((uRebindMask & static_cast<uint>(ResourceRebindFlags::READ_TEXTURES)) > 0)
      {
        m_pImpl->_bindReadTextures(static_cast<ShaderStage>(iShaderStage),
                                   m_clResourceState.pBoundReadTextures[iShaderStage],
                                   FANCY_MAX_NUM_BOUND_READ_TEXTURES);
      }
      if((uRebindMask & static_cast<uint>(ResourceRebindFlags::READ_BUFFERS)) > 0)
      {
        m_pImpl->_bindReadBuffers(static_cast<ShaderStage>(iShaderStage),
                                  m_clResourceState.pBoundReadBuffers[iShaderStage],
                                  FANCY_MAX_NUM_BOUND_READ_BUFFERS);
      }
      if((uRebindMask & static_cast<uint>(ResourceRebindFlags::CONSTANT_BUFFERS)) > 0)
      {
        m_pImpl->_bindConstantBuffers(static_cast<ShaderStage>(iShaderStage),
                                      m_clResourceState.pBoundConstantBuffers[iShaderStage],
                                      FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS);
      }
      if((uRebindMask & static_cast<uint>(ResourceRebindFlags::TEXTURE_SAMPLERS)) > 0)
      {
        m_pImpl->_bindTextureSamplers(static_cast<ShaderStage>(iShaderStage),
                                      m_clResourceState.pBoundTextureSamplers[iShaderStage],
                                      FANCY_MAX_NUM_BOUND_SAMPLERS);
      }
      if((uRebindMask & static_cast<uint>(ResourceRebindFlags::GPU_PROGRAMS)) > 0)
      {
        m_pImpl->_bindGPUProgram(static_cast<ShaderStage>(iShaderStage),
                                 m_clResourceState.pBoundGPUPrograms[iShaderStage]);
      }
    }
  }

  m_uPipelineRebindMask = static_cast<uint>(PipelineRebindFlags::NONE);
  m_bChangingPipelineState = false;
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setDepthTestEnabled( bool bEnabled )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getDepthTestEnabled() == bEnabled)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.bDepthTestEnabled = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setDepthWriteEnabled( bool bEnabled )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getDepthWriteEnabled() == bEnabled)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.bDepthWriteEnabled = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setDepthCompFunc( CompFunc eFunc )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getDepthCompFunc() == eFunc)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eDepthCompFunc = eFunc;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilEnabled( bool bEnabled )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilEnabled() == bEnabled)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.bStencilEnabled = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilRefValue( uint uRefValue )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilRefValue() == uRefValue)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.uStencilRef = uRefValue;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilReadMask( uint8 uReadMask )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilReadMask() == uReadMask)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.u8StencilReadMask = uReadMask;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilWriteMask( uint8 uWriteMask )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilWriteMask() == uWriteMask)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.u8StencilWriteMask = uWriteMask;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilFailOp_FF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilFailOp_FF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilFailOp_FF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilDepthFailOp_FF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilDepthFailOp_FF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_FF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilPassOp_FF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilPassOp_FF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilPassOp_FF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilCompFunc_FF( CompFunc eFunc )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilCompFunc_FF() == eFunc)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilCompFunc_FF = eFunc;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilFailOp_BF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilFailOp_BF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilFailOp_BF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilDepthFailOp_BF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilDepthFailOp_BF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_BF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilPassOp_BF( StencilOp eOp )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilPassOp_BF() == eOp)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilPassOp_BF = eOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setStencilCompFunc_BF( CompFunc eFunc )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getStencilCompFunc_BF() == eFunc)
  {
    return;
  }

  m_clPipelineState.clDepthStencilState.eStencilCompFunc_BF = eFunc;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::DEPTHSTENCIL);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setAlphaToCoverageEnabled( bool bEnabled )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getAlphaToCoverageEnabled() == bEnabled)
  {
    return;
  }

  m_clPipelineState.clBlendState.bAlphaToCoverageEnabled = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setBlendStatePerRTEnabled( bool bEnabled )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getBlendStatePerRTEnabled() == bEnabled)
  {
    return;
  }

  m_clPipelineState.clBlendState.bBlendStatePerRT = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setBlendingEnabled( bool bEnabled, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getBlendingEnabled(u8RenderTargetIndex) == bEnabled)
  {
    return;
  }

  m_clPipelineState.clBlendState.bBlendEnabled[u8RenderTargetIndex] = bEnabled;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setSrcBlendInput( BlendInput eBlendInput, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getSrcBlendInput(u8RenderTargetIndex) == eBlendInput)
  {
    return;
  }

  m_clPipelineState.clBlendState.eSrcBlend[u8RenderTargetIndex] = eBlendInput;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setDestBlendInput( BlendInput eBlendInput, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getDestBlendInput(u8RenderTargetIndex) == eBlendInput)
  {
    return;
  }

  m_clPipelineState.clBlendState.eDestBlend[u8RenderTargetIndex] = eBlendInput;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setBlendOp( BlendOp eBlendOp, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getBlendOp(u8RenderTargetIndex) == eBlendOp)
  {
    return;
  }

  m_clPipelineState.clBlendState.eBlendOp[u8RenderTargetIndex] = eBlendOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setSrcBlendAlphaInput( BlendInput eBlendInput, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getSrcBlendAlphaInput(u8RenderTargetIndex) == eBlendInput)
  {
    return;
  }

  m_clPipelineState.clBlendState.eSrcBlendAlpha[u8RenderTargetIndex] = eBlendInput;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setDestBlendAlphaInput( BlendInput eBlendInput, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getDestBlendAlphaInput(u8RenderTargetIndex) == eBlendInput)
  {
    return;
  }

  m_clPipelineState.clBlendState.eDestBlendAlpha[u8RenderTargetIndex] = eBlendInput;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setBlendOpAlpha( BlendOp eBlendOp, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getBlendOpAlpha(u8RenderTargetIndex) == eBlendOp)
  {
    return;
  }

  m_clPipelineState.clBlendState.eBlendOpAlpha[u8RenderTargetIndex] = eBlendOp;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setRenderTargetWriteMask( uint8 u8Mask, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getRenderTargetWriteMask(u8RenderTargetIndex) == u8Mask)
  {
    return;
  }

  m_clPipelineState.clBlendState.u8RTwriteMask[u8RenderTargetIndex] = u8Mask;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::BLENDING);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setFillMode( FillMode eFillMode )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getFillMode() == eFillMode)
  {
    return;
  }

  m_clPipelineState.eFillMode = eFillMode;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::FILLMODE);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setCullMode( CullMode eCullMode )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getCullMode() == eCullMode)
  {
    return;
  }

  m_clPipelineState.eCullMode = eCullMode;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::CULLMODE);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::setWindingOrder( WindingOrder eWindingOrder )
{
  ASSERT_M(m_bChangingPipelineState, "Tried to change pipeline state outside of begin/end changePipelineState()!");
  
  if(getWindingOrder() == eWindingOrder)
  {
    return;
  }

  m_clPipelineState.eWindingOrder = eWindingOrder;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::WINDINGORDER);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindRenderTarget( Texture* pRTTexture, uint8 u8RenderTargetIndex )
{
  ASSERT_M(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS, "Referenced an undefined renderTarget");
  ASSERT_M(m_bChangingResourceState, "Tried to change pipeline state outside of begin/end changePipelineState()!");

  if(getBoundRenderTarget(u8RenderTargetIndex) == pRTTexture)
  {
    return;
  }

  m_clPipelineState.pBoundRenderTargets[u8RenderTargetIndex] = pRTTexture;
  m_uPipelineRebindMask |= static_cast<uint>(PipelineRebindFlags::RENDERTARGETS);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindReadTexture( Texture* pTexture, ShaderStage eShaderStage, uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_READ_TEXTURES, "Referenced an undefined texture register");
  ASSERT_M(m_bChangingResourceState, "Tried to change resource state outside of begin/end changeResourceState()!");

  if(m_clResourceState.pBoundReadTextures[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pTexture)
  {
    return;
  }

  m_clResourceState.pBoundReadTextures[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pTexture;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_TEXTURES);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindReadBuffer( Buffer* pBuffer, ShaderStage eShaderStage, uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_READ_BUFFERS, "Referenced an undefined buffer register");
  ASSERT_M(m_bChangingResourceState, "Tried to change resource state outside of begin/end changeResourceState()!");

  if(m_clResourceState.pBoundReadBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pBuffer)
  {
    return;
  }

  m_clResourceState.pBoundReadBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pBuffer;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::READ_BUFFERS);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindConstantBuffer( ConstantBuffer* pConstantBuffer, ShaderStage eShaderStage, uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS, "Referenced an undefined constant buffer register");
  ASSERT_M(m_bChangingResourceState, "Tried to change resource state outside of begin/end changeResourceState()!");

  if(m_clResourceState.pBoundConstantBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pConstantBuffer)
  {
    return;
  }

  m_clResourceState.pBoundConstantBuffers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pConstantBuffer;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::CONSTANT_BUFFERS);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindTextureSampler( TextureSampler* pSampler, ShaderStage eShaderStage, uint8 u8RegisterIndex )
{
  ASSERT_M(u8RegisterIndex < FANCY_MAX_NUM_BOUND_SAMPLERS, "Referenced an undefined sampler register");
  ASSERT_M(m_bChangingResourceState, "Tried to change resource state outside of begin/end changeResourceState()!");

  if(m_clResourceState.pBoundTextureSamplers[static_cast<uint>(eShaderStage)][u8RegisterIndex] == pSampler)
  {
    return;
  }

  m_clResourceState.pBoundTextureSamplers[static_cast<uint>(eShaderStage)][u8RegisterIndex] = pSampler;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::TEXTURE_SAMPLERS);
}
//////////////////////////////////////////////////////////////////////////
void Renderer::bindGPUProgram( GPUProgram* pProgram, ShaderStage eShaderStage )
{
  ASSERT_M(m_bChangingResourceState, "Tried to change resource state outside of begin/end changeResourceState()!");

  if(m_clResourceState.pBoundGPUPrograms[static_cast<uint>(eShaderStage)] == pProgram)
  {
    return;
  }

  m_clResourceState.pBoundGPUPrograms[static_cast<uint>(eShaderStage)] = pProgram;
  m_uResourceRebindMask[static_cast<uint>(eShaderStage)] |= static_cast<uint>(ResourceRebindFlags::GPU_PROGRAMS);
}
//////////////////////////////////////////////////////////////////////////