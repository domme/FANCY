#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "FancyCorePrerequisites.h"
#include "LoadableObject.h"


// Common defines for the RenderSystem
// TODO: Check these values again...
#define FANCY_MAX_NUM_BOUND_RENDERTARGETS 8
#define FANCY_MAX_NUM_BOUND_SAMPLERS 16
#define FANCY_MAX_NUM_BOUND_READ_BUFFERS 32
#define FANCY_MAX_NUM_BOUND_READ_TEXTURES 32
#define FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS 8 

namespace FANCY { namespace Core { namespace Rendering {

namespace CompFunc { enum Enum {
  NEVER = 0,
  LESS,
  EQUAL,
  LEQUAL,
  GREATER,
  NOTEQUAL,
  GEQUAL,
  ALWAYS,

  NUM
}; }

namespace StencilOp { enum Enum {
  KEEP = 0,
  ZERO,
  REPLACE,
  INCREMENT_CLAMP,
  DECREMENT_CLAMP,
  INVERT,
  INCEMENT_WRAP,
  DECREMENT_WRAP,

  NUM
}; }

namespace FillMode { enum Enum {
  WIREFRAME = 0,
  SOLID,

  NUM
}; }

namespace CullMode { enum Enum {
  NONE = 0,
  FRONT,
  BACK,

  NUM
}; }

namespace WindingOrder { enum Enum {
  CCW = 0,
  CW,

  NUM
}; }

namespace BlendInput { enum Enum {
  ZERO = 0,
  ONE,
  SRC_COLOR,
  INV_SRC_COLOR,
  SRC_ALPHA,
  INV_SRC_ALPHA,
  DEST_ALPHA,
  INV_DEST_ALPHA,
  DEST_COLOR,
  INV_DEST_COLOR,
  SRC_ALPHA_CLAMPED,
  BLEND_FACTOR,  // DX11-only
  INV_BLEND_FACTOR,
  SRC1_COLOR,
  INV_SRC1_COLOR,
  SRC1_ALPHA,
  INV_SRC1_ALPHA,

  // OpenGL-only:
  CONSTANT_COLOR,
  INV_CONSTANT_COLOR,
  CONSTANT_ALPHA,
  INV_CONSTANT_ALPHA,

  NUM
}; }

namespace BlendOp { enum Enum {
  ADD = 0,
  SUBTRACT,
  REV_SUBTRACT,
  MIN,
  MAX,

  NUM
}; }

namespace ShaderStage { enum Enum {
  VERTEX        = 0,
  FRAGMENT,      
  GEOMETRY,      
  TESS_HULL,     
  TESS_DOMAIN,   
  COMPUTE,                  
  
  NUM
}; }

struct DepthStencilState {
  bool              bDepthTestEnabled;
  bool              bDepthWriteEnabled;
  CompFunc::Enum    eDepthCompFunc;

  bool              bStencilEnabled;
  uint              uStencilRef;
  uint8             u8StencilReadMask;
  uint8             u8StencilWriteMask;
  StencilOp::Enum   eStencilFailOp_FF;
  StencilOp::Enum   eStencilDepthFailOp_FF;
  StencilOp::Enum   eStencilPassOp_FF;
  CompFunc::Enum    eStencilCompFunc_FF;
  StencilOp::Enum   eStencilFailOp_BF;
  StencilOp::Enum   eStencilDepthFailOp_BF;
  StencilOp::Enum   eStencilPassOp_BF;
  CompFunc::Enum    eStencilCompFunc_BF;
};

struct BlendState { 
  bool              bAlphaToCoverageEnabled;
  bool              bBlendStatePerRT;

  bool              bBlendEnabled     [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput::Enum  eSrcBlend         [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput::Enum  eDestBlend        [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendOp::Enum     eBlendOp          [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput::Enum  eSrcBlendAlpha    [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput::Enum  eDestBlendAlpha   [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendOp::Enum     eBlendOpAlpha     [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  uint8             u8RTwriteMask     [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
};

// Forward-declarations of needed classes
class Mesh;
class VolumeMesh;
class Material;
class RenderOperation;
class Texture;
class TextureSampler;
class Buffer;
class ConstantBuffer;
class GPUProgram;

/// Polymorphic implementation interface (for different render-platforms)
class RendererImpl : public LoadableObject
{
public:
  RendererImpl();
  virtual ~RendererImpl();
  
  virtual void _setDepthStencilState(const DepthStencilState& clState) = 0;
  virtual void _setFillMode(FillMode::Enum eFillMode) = 0;
  virtual void _setCullMode(CullMode::Enum eCullMode) = 0;
  virtual void _setWindingOrder(WindingOrder::Enum eWindingOrder) = 0;
  virtual void _setBlendState(const BlendState& clState) = 0;
      
  // TODO: Mesh will become a thin geometric representation in the future
  virtual void _render(Mesh* pMesh) = 0;  
  virtual void _renderIndirect( /*params ?*/ ) = 0;
  virtual void _dispatchCompute( /*params ?*/ ) = 0;
  virtual void _dispatchComputeIndirect( /*params ?*/ ) = 0;

  /// Resource-bindings
  virtual void _bindRenderTargets(Texture** pTexList, uint8 u8NumRTs) = 0;
  virtual void _bindReadTextures(ShaderStage::Enum eShaderStage,
                                 const Texture** pTexList,
                                 uint8 u8NumTextures) = 0;
  virtual void _bindReadBuffers(ShaderStage::Enum eShaderStage,
                                const Buffer** pBufferList,
                                uint8 u8NumBuffers) = 0;
  virtual void _bindConstantBuffers(ShaderStage::Enum eShaderStage,
                                    const ConstantBuffer** pBufferList,
                                    uint8 u8NumBuffers) = 0;
  virtual void _bindTextureSamplers(ShaderStage::Enum eShaderStage,
                                    const TextureSampler** pTexSamplerList,
                                    uint8 u8NumTexSamplers) = 0;
  virtual void _bindGPUProgram(ShaderStage::Enum  eShaderStage, const GPUProgram* pProgram) = 0;
};


struct PipelineState
{
  DepthStencilState   clDepthStencilState;
  BlendState          clBlendState;
  FillMode::Enum      eFillMode;
  CullMode::Enum      eCullMode;
  WindingOrder::Enum  eWindingOrder;
  Texture* pBoundRenderTargets [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
};

struct ResourceState
{
  Texture* pBoundReadTextures [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_TEXTURES];
  Buffer* pBoundReadBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_BUFFERS];
  ConstantBuffer* pBoundConstantBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS];
  TextureSampler* pBoundTextureSamplers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_SAMPLERS];
  GPUProgram*     pBoundGPUPrograms [ShaderStage::NUM];
};

// TODO: Could be optimized by tracking the individual components of depthStencilstate, BlendState
//       or renderTargets and not rebind them all.
namespace PipelineRebindFlags { enum Enum {
  NONE          = 0x0000,
  DEPTHSTENCIL  = 0x0001,  
  BLENDING      = 0x0002,
  FILLMODE      = 0x0004,
  CULLMODE      = 0x0008,
  WINDINGORDER  = 0x0010,
  RENDERTARGETS = 0x0020,
  ALL           = 0xFFFF
}; }

namespace ResourceRebindFlags { enum Enum {
  NONE              = 0x0000,
  READ_TEXTURES     = 0x0001,
  READ_BUFFERS      = 0x0002,
  CONSTANT_BUFFERS  = 0x0004,
  TEXTURE_SAMPLERS  = 0x0008,
  GPU_PROGRAMS      = 0x0010,
  ALL               = 0xFFFF
}; }

class DLLEXPORT Renderer : public LoadableObject
{
public:
  bool setImplementation(RendererImpl* p_Inst);
  static Renderer& getInstance() { static Renderer instance; return instance; }
  virtual ~Renderer();

  virtual bool _init() override;
  virtual bool _destroy() override;

  /// Notifies that a bunch of pipeline-changes are incoming
  void beginChangePipelineState();
  /// All pipeline-changes are completed for now. Apply it to the implementation
  void endChangePipelineState();
  /// Notifies that a series of resource-bindings will be issued
  void beginChangeResourceState();
  /// Binds all changed resources to the pipeline
  void endChangeResourceState();

  /// Getter/setters to modify depthStencilState. Setters are only allowed between begin/endChangePipelineState
  void setDepthTestEnabled(bool bEnabled);
  bool getDepthTestEnabled() const { return m_clPipelineState.clDepthStencilState.bDepthTestEnabled; }
  
  void setDepthWriteEnabled(bool bEnabled);
  bool getDepthWriteEnabled() const { return m_clPipelineState.clDepthStencilState.bDepthWriteEnabled; }

  void setDepthCompFunc(CompFunc::Enum eFunc);
  CompFunc::Enum getDepthCompFunc() const { return m_clPipelineState.clDepthStencilState.eDepthCompFunc; }
  
  void setStencilEnabled(bool bEnabled);
  bool getStencilEnabled() const { return m_clPipelineState.clDepthStencilState.bStencilEnabled; }

  void setStencilRefValue(uint uRefValue);
  uint getStencilRefValue() const { return m_clPipelineState.clDepthStencilState.uStencilRef; }

  void setStencilReadMask(uint8 uReadMask);
  uint8 getStencilReadMask() const { return m_clPipelineState.clDepthStencilState.u8StencilReadMask; }

  void setStencilWriteMask(uint8 uWriteMask);
  uint8 getStencilWriteMask() const { return m_clPipelineState.clDepthStencilState.u8StencilWriteMask; }
  
  void setStencilFailOp_FF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilFailOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilFailOp_FF; }

  void setStencilDepthFailOp_FF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilDepthFailOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_FF; }

  void setStencilPassOp_FF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilPassOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilPassOp_FF; }

  void setStencilCompFunc_FF(CompFunc::Enum eFunc);
  CompFunc::Enum getStencilCompFunc_FF() const { return m_clPipelineState.clDepthStencilState.eStencilCompFunc_FF; }

  void setStencilFailOp_BF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilFailOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilFailOp_BF; }

  void setStencilDepthFailOp_BF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilDepthFailOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_BF; }

  void setStencilPassOp_BF(StencilOp::Enum eOp);
  StencilOp::Enum getStencilPassOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilPassOp_BF; }

  void setStencilCompFunc_BF(CompFunc::Enum eFunc);
  CompFunc::Enum getStencilCompFunc_BF() const { return m_clPipelineState.clDepthStencilState.eStencilCompFunc_BF; }
  /// end depthStencilState getter/setters

  /// getters/setters to modify blendState. Setters are only allowed between begin/endChangePipelineState
  void setAlphaToCoverageEnabled(bool bEnabled);
  bool getAlphaToCoverageEnabled() const { return m_clPipelineState.clBlendState.bAlphaToCoverageEnabled; }

  void setBlendStatePerRTEnabled(bool bEnabled);
  bool getBlendStatePerRTEnabled() const { return m_clPipelineState.clBlendState.bBlendStatePerRT; }

  void setBlendingEnabled(bool bEnabled, uint8 u8RenderTargetIndex);
  bool getBlendingEnabled(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.bBlendEnabled[u8RenderTargetIndex]; }
  
  void setSrcBlendInput(BlendInput::Enum eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput::Enum getSrcBlendInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eSrcBlend[u8RenderTargetIndex]; }

  void setDestBlendInput(BlendInput::Enum eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput::Enum getDestBlendInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eDestBlend[u8RenderTargetIndex]; }

  void setBlendOp(BlendOp::Enum eBlendOp, uint8 u8RenderTargetIndex);
  BlendOp::Enum getBlendOp(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eBlendOp[u8RenderTargetIndex]; }

  void setSrcBlendAlphaInput(BlendInput::Enum eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput::Enum getSrcBlendAlphaInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eSrcBlendAlpha[u8RenderTargetIndex]; }

  void setDestBlendAlphaInput(BlendInput::Enum eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput::Enum getDestBlendAlphaInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eDestBlendAlpha[u8RenderTargetIndex]; }

  void setBlendOpAlpha(BlendOp::Enum eBlendOp, uint8 u8RenderTargetIndex);
  BlendOp::Enum getBlendOpAlpha(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eBlendOpAlpha[u8RenderTargetIndex]; }

  void setRenderTargetWriteMask(uint8 u8Mask, uint8 u8RenderTargetIndex);
  uint8 getRenderTargetWriteMask(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.u8RTwriteMask[u8RenderTargetIndex]; }
  /// end blendState setter/getter
  
  void setFillMode(FillMode::Enum eFillMode);
  FillMode::Enum getFillMode() const { return m_clPipelineState.eFillMode; }

  void setCullMode(CullMode::Enum eCullMode);
  CullMode::Enum getCullMode() const { return m_clPipelineState.eCullMode; }

  void setWindingOrder(WindingOrder::Enum eWindingOrder);
  WindingOrder::Enum getWindingOrder() const { return m_clPipelineState.eWindingOrder; }
  
  void bindRenderTarget(Texture* pRTTexture, uint8 u8RenderTargetIndex);
  Texture* getBoundRenderTarget(uint8 u8RenderTargetIndex) const 
   { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.pBoundRenderTargets[u8RenderTargetIndex]; }

  void bindReadTexture(Texture* pTexture, ShaderStage::Enum eShaderStage, uint8 u8RegisterIndex);
  void bindReadBuffer(Buffer* pBuffer, ShaderStage::Enum eShaderStage, uint8 u8RegisterIndex);
  void bindConstantBuffer(ConstantBuffer* pConstantBuffer, ShaderStage::Enum eShaderStage, uint8 u8RegisterIndex);
  void bindTextureSampler(TextureSampler* pSampler, ShaderStage::Enum eShaderStage, uint8 u8RegisterIndex);
  void bindGPUProgram(GPUProgram* pProgram, ShaderStage::Enum eShaderStage);
  
protected:
  Renderer();
  /// Pointer to the actual platform-dependent implementation
  RendererImpl* m_pImpl;
  /// The current cached state of the pipeline
  PipelineState m_clPipelineState;
  /// The current cached state of the bound resources
  ResourceState m_clResourceState;
  /// Mask indicating which pipeline states have to be re-bound to the pipeline
  uint        m_uPipelineRebindMask;
  /// Mask indicating which resources have to be re-bound to each shaderStage
  uint        m_uResourceRebindMask[ShaderStage::NUM];
  /// Indicates if the current code-block is between begin/end ChangePipelineState;
  bool        m_bChangingPipelineState;
  /// Indicates if the current code-block is between begin/end ChangeResourceState;
  bool        m_bChangingResourceState;
};

} // end of namespace Rendering
} // end of namespace Core
} // end of namespace FANCY

#endif