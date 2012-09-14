#ifndef GLBUFFERMANAGER_H
#define GLBUFFERMANAGER_H

#include "../../includes.h"

class GLBufferUploader
{
public:
	~GLBufferUploader();
	static GLBufferUploader& GetInstance() { static GLBufferUploader instance; return instance; }
		
	GLuint UploadBufferData( void* pData, uint32 u32NumElements, uint16 u16Stride, GLuint uGLbufferType, GLuint uGLstorageHint );

private:
	GLBufferUploader();
};

#endif