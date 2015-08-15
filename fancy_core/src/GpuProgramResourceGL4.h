#ifndef INCLUDE_GPUPROGRAMRESOURCEGL4_H
#define INCLUDE_GPUPROGRAMRESOURCEGL4_H

#include "OpenGLprerequisites.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy {namespace Rendering { namespace GL4 {
  class GpuProgramResourceGL4 {
    public:
      /// e.g. GL_TEXTURE_1D/2D/3D for textures
      GLenum bindingTargetGL;
      /// e.g. GL_RGB32F 
      GLenum dataFormatGL;

      void serialize(IO::Serializer* aSerializer);
  };
} } } // end of namespace Fancy::Rendering::GL4

#endif // INCLUDE_GPUPROGRAMRESOURCEGL4_H