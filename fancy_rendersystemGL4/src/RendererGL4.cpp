#include "RendererGL4.h"

using namespace FANCY::Core::Rendering;

RendererGL4::RendererGL4()
{

}

RendererGL4::~RendererGL4()
{

}

bool RendererGL4::_init()
{
  glewInit();
}

bool RendererGL4::_destroy()
{
  // TODO: Gather all GL-resources here and destroy them? Or simply destroy internal stuff?
}

void RendererGL4::_setDepthStencilState( const DepthStencilState& clState )
{
  
}

void RendererGL4::_setFillMode( FillMode eFillMode )
{

}

void RendererGL4::_setCullMode( CullMode eCullMode )
{

}

void RendererGL4::_setWindingOrder( WindingOrder eWindingOrder )
{

}

void RendererGL4::_setBlendState( const BlendState& clState )
{

}

void RendererGL4::_render( Mesh* pMesh )
{

}

void RendererGL4::_renderIndirect( /*params ?*/ )
{

}

void RendererGL4::_dispatchCompute( /*params ?*/ )
{

}

void RendererGL4::_dispatchComputeIndirect( /*params ?*/ )
{

}

void RendererGL4::_bindRenderTargets( Texture** pTexList, uint8 u8NumRTs )
{

}

void RendererGL4::_bindReadTextures( ShaderStage eShaderStage, const Texture** pTexList, uint8 u8NumTextures )
{

}

void RendererGL4::_bindReadBuffers( ShaderStage eShaderStage, const Buffer** pBufferList, uint8 u8NumBuffers )
{

}

void RendererGL4::_bindConstantBuffers( ShaderStage eShaderStage, const ConstantBuffer** pBufferList, uint8 u8NumBuffers )
{

}

void RendererGL4::_bindTextureSamplers( ShaderStage eShaderStage, const TextureSampler** pTexSamplerList, uint8 u8NumTexSamplers )
{

}

void RendererGL4::_bindGPUProgram( ShaderStage eShaderStage, const GPUProgram* pProgram )
{

}
