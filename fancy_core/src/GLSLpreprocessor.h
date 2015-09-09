#ifndef INCLUDE_GLSLPREPROCESSOR_H
#define INCLUDE_GLSLPREPROCESSOR_H

#include "FancyCorePrerequisites.h"

#if defined (RENDERER_OPENGL4)

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GLSLpreprocessor
  {
    public: 
      GLSLpreprocessor();
      virtual ~GLSLpreprocessor();
  
      void preprocessShader( String& rFinalShaderString, const String& szPath );
  
    private:
      
  };
//---------------------------------------------------------------------------//
} } } // end of namespcae Fancy::Rendering::GL4

#endif
#endif  // INCLUDE_GLSLPREPROCESSOR_H