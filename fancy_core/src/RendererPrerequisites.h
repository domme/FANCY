#ifndef INCLUDE_RENDERERPREREQUISITES
#define INCLUDE_RENDERERPREREQUISITES

// This define selects the render-system
#define RENDERER_OPENGL4
//#define RENDERER_DX11


// Common defines for the RenderSystem
// TODO: Check these values again...
#define FANCY_MAX_NUM_RENDERTARGETS 8
#define FANCY_MAX_NUM_BOUND_SAMPLERS 16
#define FANCY_MAX_NUM_BOUND_READ_BUFFERS 32
#define FANCY_MAX_NUM_BOUND_READ_TEXTURES 32
#define FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS 8 

#if defined (RENDERER_OPENGL4)
#include "OpenGLprerequisites.h"
#endif // RENDERER_OPENGL4

namespace FANCY { namespace Core { namespace Rendering {

#if defined (RENDERER_OPENGL4)
// Platform-dependent forward declarations
namespace GL4 {
  class RendererGL4;
}
#define PLATFORM_DEPENDENT_NAME(name) FANCY::Core::Rendering::GL4::name##GL4
#elif defined (RENDERER_DX11)
  // TODO: IMPLEMENT
#endif // RENDERER

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
//-----------------------------------------------------------------------//
enum class PipelineRebindFlags {
  NONE                        = 0x0000,
  DEPTHSTENCIL                = 0x0001,
  BLENDING                    = 0x0002,
  FILLMODE                    = 0x0004,
  CULLMODE                    = 0x0008,
  WINDINGORDER                = 0x0010,
  RENDERTARGETS               = 0x0020,
  COLOR_WRITE_MASK            = 0x0040,
  ALL                         = 0xFFFF
};
//-----------------------------------------------------------------------//
enum class ResourceRebindFlags {
	NONE              = 0x0000,
	READ_TEXTURES     = 0x0001,
	READ_BUFFERS      = 0x0002,
	CONSTANT_BUFFERS  = 0x0004,
	TEXTURE_SAMPLERS  = 0x0008,
	GPU_PROGRAMS      = 0x0010,
	ALL               = 0xFFFF
};
//-----------------------------------------------------------------------//
enum class FaceType {
	FRONT = 0,
	BACK,

	NUM
};
//-----------------------------------------------------------------------//
class DepthStencilState {
  friend class DepthStencilStateManager;
  friend class PLATFORM_DEPENDENT_NAME(Renderer);

  public:
    bool operator==(const DepthStencilState& clOther) const;

  private:
    DepthStencilState() {}
    ~DepthStencilState() {}

    uint32            u32Hash;
    bool              bDepthTestEnabled;
    bool              bDepthWriteEnabled;
    CompFunc          eDepthCompFunc;

    bool              bStencilEnabled;
    bool              bTwoSidedStencil;
    int               iStencilRef;
    uint32            uStencilReadMask;
    CompFunc          eStencilCompFunc[FaceType::NUM];
    uint32            uStencilWriteMask[FaceType::NUM];
    StencilOp         eStencilFailOp[FaceType::NUM];
    StencilOp         eStencilDepthFailOp[FaceType::NUM];
    StencilOp         eStencilPassOp[FaceType::NUM];
};
//-----------------------------------------------------------------------//
class BlendState {
    friend class DepthStencilStateManager;
    friend class PLATFORM_DEPENDENT_NAME(Renderer);

  public:
    bool operator==(const BlendState& clOther) const;

  private:
    BlendState() {}
    ~BlendState() {}

    uint32            u32Hash;
    bool              bAlphaToCoverageEnabled;
    bool              bBlendStatePerRT;

    bool              bAlphaSeparateBlend [FANCY_MAX_NUM_RENDERTARGETS];
    bool              bBlendEnabled       [FANCY_MAX_NUM_RENDERTARGETS];
    BlendInput        eSrcBlend           [FANCY_MAX_NUM_RENDERTARGETS];
    BlendInput        eDestBlend          [FANCY_MAX_NUM_RENDERTARGETS];
    BlendOp           eBlendOp            [FANCY_MAX_NUM_RENDERTARGETS];
    BlendInput        eSrcBlendAlpha      [FANCY_MAX_NUM_RENDERTARGETS];
    BlendInput        eDestBlendAlpha     [FANCY_MAX_NUM_RENDERTARGETS];
    BlendOp           eBlendOpAlpha       [FANCY_MAX_NUM_RENDERTARGETS];
    uint32            uRTwriteMask        [FANCY_MAX_NUM_RENDERTARGETS];
};

// Forward-declarations of common rendering classes
class Mesh;
class VolumeMesh;
class Material;
class RenderOperation;
class Texture;
class TextureSampler;
class Buffer;
class ConstantBuffer;
class GPUProgram;

} } }  // end of namespace FANCY::Core::Rendering

// Platform-specific includes
//////////////////////////////////////////////////////////////////////////
#if defined (RENDERER_OPENGL4)
  #include "RendererGL4.h"
  #include "GLutil.h"  
#elif defined (RENDERER_DX11)
    #define PLATFORM_DEPENDENT_NAME(name) name##DX11
#endif

#endif  // INCLUDE_RENDERERPREREQUISITES