#ifndef INCLUDE_RENDERERPREREQUISITES
#define INCLUDE_RENDERERPREREQUISITES

// This define selects the render-system
#define RENDERER_OPENGL4
//#define RENDERER_DX11

#if defined (RENDERER_OPENGL4)
#include "OpenGLprerequisites.h"
#endif // RENDERER_OPENGL4

// Common defines for the RenderSystem
// TODO: Check these values again...
#define FANCY_MAX_NUM_RENDERTARGETS 7 // (-1 for depth-stencil target)
#define FANCY_MAX_NUM_BOUND_SAMPLERS 16
#define FANCY_MAX_NUM_BOUND_READ_BUFFERS 32
#define FANCY_MAX_NUM_BOUND_READ_TEXTURES 32
#define FANCY_MAX_NUM_BOUND_CONSTANT_BUFFERS 8 

//---------------------------------------------------------------------------//
namespace FANCY { namespace Core { namespace Rendering {

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
//---------------------------------------------------------------------------//
// Forward-declarations of platform-dependent rendering classes
#if defined (RENDERER_OPENGL4)
  namespace GL4 
  {
    class RendererGL4;
    class TextureGL4;
    class GPUProgramGL4;
  }
  #define PLATFORM_DEPENDENT_NAME(name) FANCY::Core::Rendering::GL4::name##GL4
  #define PLATFORM_DEPENDENT_INCLUDE_RENDERER   "RendererGL4.h"
  #define PLATFORM_DEPENDENT_INCLUDE_TEXTURE    "TextureGL4.h"
  #define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAM "GPUProgramGL4.h"
#elif defined (RENDERER_DX11)
  namespace DX11 {}
  #define PLATFORM_DEPENDENT_NAME(name) FANCY::Core::Rendering::GL4::name##DX11
#endif // RENDERER
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------//
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
//-----------------------------------------------------------------------//
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
//-----------------------------------------------------------------------//
enum class FillMode {
  WIREFRAME = 0,
  SOLID,

  NUM
};
//-----------------------------------------------------------------------//
enum class CullMode { 
  NONE = 0,
  FRONT,
  BACK,

  NUM
};
//-----------------------------------------------------------------------//
enum class WindingOrder {
  CCW = 0,
  CW,

  NUM
};
//-----------------------------------------------------------------------//    
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
//---------------------------------------------------------------------------//
enum class BlendOp {
  ADD = 0,
  SUBTRACT,
  REV_SUBTRACT,
  MIN,
  MAX,

  NUM
};
//---------------------------------------------------------------------------//
enum class ShaderStage {
  VERTEX        = 0,
  FRAGMENT,      
  GEOMETRY,      
  TESS_HULL,     
  TESS_DOMAIN,   
  COMPUTE,                  

  NUM
};
//---------------------------------------------------------------------------//
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
  DepthStencilState();
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
  BlendState();
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
//---------------------------------------------------------------------------//

} } }  // end of namespace FANCY::Core::Rendering

#endif  // INCLUDE_RENDERERPREREQUISITES