#ifndef FBOSERVICE_H
#define FBOSERVICE_H

#include "../Includes.h"

#include <assert.h>

class FBOservice
{
public:
	static void checkFBOErrors()
	{
		GLenum error = glCheckFramebufferStatus( GL_FRAMEBUFFER );

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

		assert( error == GL_FRAMEBUFFER_COMPLETE );
	}

};

#endif