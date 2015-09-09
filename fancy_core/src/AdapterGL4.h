#ifndef INCLUDE_ADAPTERGL4_H
#define INCLUDE_ADAPTERGL4_H

#include "RendererPrerequisites.h"
#include "GpuProgramResource.h"

#if defined (RENDERER_OPENGL4)

namespace Fancy { namespace Rendering { namespace GL4 {
  
  class Adapter {
  public:
    //---------------------------------------------------------------------------//
      static GLenum mapResourceTypeToGLbindingTarget(const GpuResourceType& generalType);
      static GLenum toGLType(const CompFunc& generalType);
      static GLenum toGLType(const StencilOp& generalType);
      static GLenum toGLType(const FillMode& generalType);
      static GLenum toGLType(const CullMode& generalType);
      static GLenum toGLType(const WindingOrder& generalType);
      static GLenum toGLType(const BlendInput& generalType);
      static GLenum toGLType(const BlendOp& generalType);
      static GLenum toGLType(const ShaderStage& generalType);
      static GLuint toGLFlag(const ShaderStageFlag& generalType);
      static GLenum toGLType(const DataFormat& generalType);
      static void mapGLpixelFormats(const DataFormat& generalPixelFormat, bool isDepthStencilFormat,
        GLenum& eFormat, GLenum& eInternalFormat, GLenum& ePixelType);
      static void getGLtypeAndNumComponentsFromFormat(const DataFormat& eFormat, 
        uint32& ruComponents, GLenum& reTypeGL);
//---------------------------------------------------------------------------//
  }; 

} } } // end of namespace Fancy::Rendering::GL4

#endif // RENDERER_GL4


#endif  // INCLUDE_ADAPTERGL4_H