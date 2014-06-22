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

enum class CompFunc {
  NEVER = 0,
  LESS,
  EQUAL,
  LEQUAL,
  GREATER,
  NOTEQUAL,
  GEQUAL,
  ALWAYS,

  NUM
};

enum class StencilOp {
  KEEP = 0,
  ZERO,
  REPLACE,
  INCREMENT_CLAMP,
  DECREMENT_CLAMP,
  INVERT,
  INCEMENT_WRAP,
  DECREMENT_WRAP,

  NUM
};

enum class FillMode {
  WIREFRAME = 0,
  SOLID,

  NUM
};

enum class CullMode { 
  NONE = 0,
  FRONT,
  BACK,

  NUM
};

enum class WindingOrder {
  CCW = 0,
  CW,

  NUM
};

enum class BlendInput {
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
};

enum class BlendOp {
  ADD = 0,
  SUBTRACT,
  REV_SUBTRACT,
  MIN,
  MAX,

  NUM
};

enum class ShaderStage {
  VERTEX        = 0,
  FRAGMENT,      
  GEOMETRY,      
  TESS_HULL,     
  TESS_DOMAIN,   
  COMPUTE,                  
  
  NUM
};

enum class ShaderStageFlag {
  VERTEX        = 0x01,
  FRAGMENT      = 0x02,      
  GEOMETRY      = 0x04,      
  TESS_HULL     = 0x08,     
  TESS_DOMAIN   = 0x10,   
  COMPUTE       = 0x20,                  

  ALL           = 0xFF
};

// TODO: Could be optimized by tracking the individual components of depthStencilstate, BlendState
//       or renderTargets and not rebind them all.
/*enum class DepthStencilRebindFlags {
  NONE                     = 0x0000,
  DEPTH_TEST_ENABLED       = 0x0001,  
  DEPTH_WRITE_ENABLED      = 0x0002,
  DEPTH_COMP_FUNC          = 0x0004,
  STENCIL_ENABLED          = 0x0008,
  STENCIL_REF              = 0x0010,
  STENCIL_READ_MASK        = 0x0020,
  STENCIL_WRITE_MASK       = 0x0040,
  STENCIL_FAIL_OP_FF       = 0x0080,
  STENCIL_DEPTH_FAIL_OP_FF = 0x0100,
  STENCIL_PASS_OP_FF       = 0x0200,
  STENCIL_COMP_FUNC_FF     = 0x0400,
  STENCIL_FAIL_OP_BF       = 0x0800,
  STENCIL_DEPTH_FAIL_OP_BF = 0x1000,
  STENCIL_PASS_OP_BF       = 0x2000,
  STENCIL_COMP_FUNC_BF     = 0x4000,
  ALL                      = 0xFFFF
};

enum class BlendingRebindFlags {
  NONE                    = 0x0000,

};*/

enum class PipelineRebindFlags {
  NONE                        = 0x0000,
  DEPTHSTENCIL                = 0x0001,
  BLENDING                    = 0x0002,
  FILLMODE                    = 0x0004,
  CULLMODE                    = 0x0008,
  WINDINGORDER                = 0x0010,
  RENDERTARGETS               = 0x0020,
  ALL                         = 0xFFFF
};


struct DepthStencilState {
  bool              bDepthTestEnabled;
  bool              bDepthWriteEnabled;
  CompFunc          eDepthCompFunc;

  bool              bStencilEnabled;
  uint              uStencilRef;
  uint8             u8StencilReadMask;
  uint8             u8StencilWriteMask;
  StencilOp         eStencilFailOp_FF;
  StencilOp         eStencilDepthFailOp_FF;
  StencilOp         eStencilPassOp_FF;
  CompFunc          eStencilCompFunc_FF;
  StencilOp         eStencilFailOp_BF;
  StencilOp         eStencilDepthFailOp_BF;
  StencilOp         eStencilPassOp_BF;
  CompFunc          eStencilCompFunc_BF;
};

struct BlendState { 
  bool              bAlphaToCoverageEnabled;
  bool              bBlendStatePerRT;

  bool              bBlendEnabled     [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput        eSrcBlend         [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput        eDestBlend        [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendOp           eBlendOp          [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput        eSrcBlendAlpha    [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendInput        eDestBlendAlpha   [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  BlendOp           eBlendOpAlpha     [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
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
  
  virtual void _onDepthStencilStateChanged(const DepthStencilState& clState, const DepthStencilState& clOldState) = 0;
  virtual void _onBlendStateChange(const BlendState& clState, const BlendState& clOldState) = 0;
  virtual void _setFillMode(FillMode eFillMode) = 0;
  virtual void _setCullMode(CullMode eCullMode) = 0;
  virtual void _setWindingOrder(WindingOrder eWindingOrder) = 0;
        
  // TODO: Mesh will become a thin geometric representation in the future
  virtual void _render(Mesh* pMesh) = 0;  
  virtual void _renderIndirect( /*params ?*/ ) = 0;
  virtual void _dispatchCompute( /*params ?*/ ) = 0;
  virtual void _dispatchComputeIndirect( /*params ?*/ ) = 0;

  /// Resource-bindings
  virtual void _onRenderTargetsChanged(Texture** pTexList, uint8 u8NumRTs) = 0;
  virtual void _bindReadTextures(ShaderStage eShaderStage,
                                 const Texture** pTexList,
                                 uint8 u8NumTextures) = 0;
  virtual void _bindReadBuffers(ShaderStage eShaderStage,
                                const Buffer** pBufferList,
                                uint8 u8NumBuffers) = 0;
  virtual void _bindConstantBuffers(ShaderStage eShaderStage,
                                    const ConstantBuffer** pBufferList,
                                    uint8 u8NumBuffers) = 0;
  virtual void _bindTextureSamplers(ShaderStage eShaderStage,
                                    const TextureSampler** pTexSamplerList,
                                    uint8 u8NumTexSamplers) = 0;
  virtual void _bindGPUProgram(ShaderStage  eShaderStage, const GPUProgram* pProgram) = 0;
};

struct PipelineState
{
  DepthStencilState   clDepthStencilState;
  BlendState          clBlendState;
  FillMode      eFillMode;
  CullMode      eCullMode;
  WindingOrder  eWindingOrder;
  Texture* pBoundRenderTargets [FANCY_MAX_NUM_BOUND_RENDERTARGETS];
  uint32 uRenderTargetBindMask;
};

enum class ResourceRebindFlags {
  NONE              = 0x0000,
  READ_TEXTURES     = 0x0001,
  READ_BUFFERS      = 0x0002,
  CONSTANT_BUFFERS  = 0x0004,
  TEXTURE_SAMPLERS  = 0x0008,
  GPU_PROGRAMS      = 0x0010,
  ALL               = 0xFFFF
};

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

  void setDepthCompFunc(CompFunc eFunc);
  CompFunc getDepthCompFunc() const { return m_clPipelineState.clDepthStencilState.eDepthCompFunc; }
  
  void setStencilEnabled(bool bEnabled);
  bool getStencilEnabled() const { return m_clPipelineState.clDepthStencilState.bStencilEnabled; }

  void setStencilRefValue(uint uRefValue);
  uint getStencilRefValue() const { return m_clPipelineState.clDepthStencilState.uStencilRef; }

  void setStencilReadMask(uint8 uReadMask);
  uint8 getStencilReadMask() const { return m_clPipelineState.clDepthStencilState.u8StencilReadMask; }

  void setStencilWriteMask(uint8 uWriteMask);
  uint8 getStencilWriteMask() const { return m_clPipelineState.clDepthStencilState.u8StencilWriteMask; }
  
  void setStencilFailOp_FF(StencilOp eOp);
  StencilOp getStencilFailOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilFailOp_FF; }

  void setStencilDepthFailOp_FF(StencilOp eOp);
  StencilOp getStencilDepthFailOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_FF; }

  void setStencilPassOp_FF(StencilOp eOp);
  StencilOp getStencilPassOp_FF() const { return m_clPipelineState.clDepthStencilState.eStencilPassOp_FF; }

  void setStencilCompFunc_FF(CompFunc eFunc);
  CompFunc getStencilCompFunc_FF() const { return m_clPipelineState.clDepthStencilState.eStencilCompFunc_FF; }

  void setStencilFailOp_BF(StencilOp eOp);
  StencilOp getStencilFailOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilFailOp_BF; }

  void setStencilDepthFailOp_BF(StencilOp eOp);
  StencilOp getStencilDepthFailOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilDepthFailOp_BF; }

  void setStencilPassOp_BF(StencilOp eOp);
  StencilOp getStencilPassOp_BF() const { return m_clPipelineState.clDepthStencilState.eStencilPassOp_BF; }

  void setStencilCompFunc_BF(CompFunc eFunc);
  CompFunc getStencilCompFunc_BF() const { return m_clPipelineState.clDepthStencilState.eStencilCompFunc_BF; }
  /// end depthStencilState getter/setters

  /// getters/setters to modify blendState. Setters are only allowed between begin/endChangePipelineState
  void setAlphaToCoverageEnabled(bool bEnabled);
  bool getAlphaToCoverageEnabled() const { return m_clPipelineState.clBlendState.bAlphaToCoverageEnabled; }

  void setBlendStatePerRTEnabled(bool bEnabled);
  bool getBlendStatePerRTEnabled() const { return m_clPipelineState.clBlendState.bBlendStatePerRT; }

  void setBlendingEnabled(bool bEnabled, uint8 u8RenderTargetIndex);
  bool getBlendingEnabled(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.bBlendEnabled[u8RenderTargetIndex]; }
  
  void setSrcBlendInput(BlendInput eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput getSrcBlendInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eSrcBlend[u8RenderTargetIndex]; }

  void setDestBlendInput(BlendInput eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput getDestBlendInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eDestBlend[u8RenderTargetIndex]; }

  void setBlendOp(BlendOp eBlendOp, uint8 u8RenderTargetIndex);
  BlendOp getBlendOp(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eBlendOp[u8RenderTargetIndex]; }

  void setSrcBlendAlphaInput(BlendInput eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput getSrcBlendAlphaInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eSrcBlendAlpha[u8RenderTargetIndex]; }

  void setDestBlendAlphaInput(BlendInput eBlendInput, uint8 u8RenderTargetIndex);
  BlendInput getDestBlendAlphaInput(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eDestBlendAlpha[u8RenderTargetIndex]; }

  void setBlendOpAlpha(BlendOp eBlendOp, uint8 u8RenderTargetIndex);
  BlendOp getBlendOpAlpha(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.eBlendOpAlpha[u8RenderTargetIndex]; }

  void setRenderTargetWriteMask(uint8 u8Mask, uint8 u8RenderTargetIndex);
  uint8 getRenderTargetWriteMask(uint8 u8RenderTargetIndex) const 
  { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.clBlendState.u8RTwriteMask[u8RenderTargetIndex]; }
  /// end blendState setter/getter
  
  void setFillMode(FillMode eFillMode);
  FillMode getFillMode() const { return m_clPipelineState.eFillMode; }

  void setCullMode(CullMode eCullMode);
  CullMode getCullMode() const { return m_clPipelineState.eCullMode; }

  void setWindingOrder(WindingOrder eWindingOrder);
  WindingOrder getWindingOrder() const { return m_clPipelineState.eWindingOrder; }
  
  void bindRenderTarget(Texture* pRTTexture, uint8 u8RenderTargetIndex);
  Texture* getBoundRenderTarget(uint8 u8RenderTargetIndex) const 
   { ASSERT(u8RenderTargetIndex < FANCY_MAX_NUM_BOUND_RENDERTARGETS); return m_clPipelineState.pBoundRenderTargets[u8RenderTargetIndex]; }

  void bindReadTexture(Texture* pTexture, ShaderStage eShaderStage, uint8 u8RegisterIndex);
  void bindReadBuffer(Buffer* pBuffer, ShaderStage eShaderStage, uint8 u8RegisterIndex);
  void bindConstantBuffer(ConstantBuffer* pConstantBuffer, ShaderStage eShaderStage, uint8 u8RegisterIndex);
  void bindTextureSampler(TextureSampler* pSampler, ShaderStage eShaderStage, uint8 u8RegisterIndex);
  void bindGPUProgram(GPUProgram* pProgram, ShaderStage eShaderStage);
  
protected:
  Renderer();
  /// Pointer to the actual platform-dependent implementation
  RendererImpl* m_pImpl;
  /// The current cached state of the pipeline
  PipelineState m_clPipelineState;
  PipelineState m_clPipelineState2;
 
  /// Pointer to the current and old resource-/pipeline-states. Swapped on each change
  PipelineState* m_pPipelineState;
  PipelineState* m_pOldPipelineState;

  /// Mask indicating which pipeline states have to be re-bound to the pipeline
  uint        m_uPipelineRebindMask;

  Texture* pBoundReadTextures [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_TEXTURES];
  uint32 uReadTextureBindMask [ShaderStage::NUM];

  Buffer* pBoundReadBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_READ_BUFFERS];
  uint32 uReadBufferBindMask [ShaderStage::NUM];

  ConstantBuffer* pBoundConstantBuffers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS];
  uint32 uConstantBufferBindMask[ShaderStage::NUM];

  TextureSampler* pBoundTextureSamplers [ShaderStage::NUM][FANCY_MAX_NUM_BOUND_SAMPLERS];
  uint32 uTextureSamplerBindMask[ShaderStage::NUM];

  GPUProgram*     pBoundGPUPrograms [ShaderStage::NUM];
  uint32 uGPUprogramBindMask;

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