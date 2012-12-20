#ifndef GLDEBUG_H
#define GLDEBUG_H

#include "../Includes.h"

class DLLEXPORT GLDebug
{
public:
	static void GL_ErrorCheckStart()
	{
		glGetError();
	}

	static void GL_ErrorCheckFinish()
	{
		GLenum err = glGetError();

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

};

#endif