#ifndef GLBUFFERMANAGER_H
#define GLBUFFERMANAGER_H

#include "../../includes.h"

class GLBufferUploader
{
public:
	~GLBufferUploader();
	static GLBufferUploader& GetInstance() { static GLBufferUploader instance; return instance; }
		
	uint32 UploadBufferData( void* pData, uint32 u32NumElements, uint16 u16Stride, uint32 uGLbufferType, uint32 uGLstorageHint );

private:
	GLBufferUploader();
};

#endif