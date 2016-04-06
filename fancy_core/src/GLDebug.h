#ifndef INCLUDE_GLDEBUG_H
#define INCLUDE_GLDEBUG_H

#include "FancyCorePrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "OpenGLprerequisites.h"

namespace Fancy { namespace Rendering {

class GLDebug
{
public:
//---------------------------------------------------------------------------//
	static void GL_ErrorCheckStart() { glGetError(); }
//---------------------------------------------------------------------------//
	static void GL_ErrorCheckEnd()
	{
		uint32 err = glGetError();

		switch( err )
		{
			case GL_INVALID_ENUM:
				{
					LOG_INFO( "GL-ERROR: INVALID ENUM" );
				}break;

			case GL_INVALID_VALUE:
				{
          LOG_INFO( "GL-ERROR: INVALID VALUE" );
				}break;

			case GL_INVALID_OPERATION:
				{
          LOG_INFO( "GL-ERROR: INVALID OPERATION" );
				}break;

			case GL_STACK_OVERFLOW:
				{
          LOG_INFO( "GL-ERROR: STACK OVERFLOW" );
				}break;

			case GL_STACK_UNDERFLOW:
				{
          LOG_INFO( "GL-ERROR: STACK UNDERFLOW" );
				}break;

			case GL_OUT_OF_MEMORY:
				{
          LOG_INFO( "GL-ERROR: OUT OF MEMORY" );
				}break;

			case GL_TABLE_TOO_LARGE:
				{
          LOG_INFO( "GL-ERROR: TABLE TOO LARGE" );
				}break;
		}
	}
//---------------------------------------------------------------------------//
  static void validateFBOcompleteness()
  {
    uint32 error = glCheckFramebufferStatus( GL_FRAMEBUFFER );

    switch( error )
    {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      LOG_INFO( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      LOG_INFO( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      LOG_INFO( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      LOG_INFO( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      LOG_INFO( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_UNSUPPORTED" );
      assert( false );
      break;
    }

    assert( error == GL_FRAMEBUFFER_COMPLETE);
  }
//---------------------------------------------------------------------------//
};

} } // end of namespace Fancy::Rendering

#endif

#endif  // INCLUDE_GLDEBUG_H