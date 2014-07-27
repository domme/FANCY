#ifndef INCLUDE_GLDEBUG_H
#define INCLUDE_GLDEBUG_H

#include "FancyCorePrerequisites.h"
#include "OpenGLprerequisites.h"

namespace FANCY { namespace Core { namespace Rendering {

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
					LOG( "GL-ERROR: INVALID ENUM" );
				}break;

			case GL_INVALID_VALUE:
				{
					LOG( "GL-ERROR: INVALID VALUE" );
				}break;

			case GL_INVALID_OPERATION:
				{
					LOG( "GL-ERROR: INVALID OPERATION" );
				}break;

			case GL_STACK_OVERFLOW:
				{
					LOG( "GL-ERROR: STACK OVERFLOW" );
				}break;

			case GL_STACK_UNDERFLOW:
				{
					LOG( "GL-ERROR: STACK UNDERFLOW" );
				}break;

			case GL_OUT_OF_MEMORY:
				{
					LOG( "GL-ERROR: OUT OF MEMORY" );
				}break;

			case GL_TABLE_TOO_LARGE:
				{
					LOG( "GL-ERROR: TABLE TOO LARGE" );
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
      LOG( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      LOG( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      LOG( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      LOG( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" );
      assert( false );
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      LOG( "FRAMEBUFFER-ERROR: GL_FRAMEBUFFER_UNSUPPORTED" );
      assert( false );
      break;
    }

    assert( error == GL_FRAMEBUFFER_COMPLETE, "FBO incomplete" );
  }
//---------------------------------------------------------------------------//
};

} } } // end of namespace FANCY::Core::Rendering

#endif  // INCLUDE_GLDEBUG_H