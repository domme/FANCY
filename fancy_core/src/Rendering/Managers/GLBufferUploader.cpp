#include "GLBufferUploader.h"

#include "GLResourcePathManager.h"


GLBufferUploader::GLBufferUploader()
{

}

GLBufferUploader::~GLBufferUploader()
{

}

uint32 GLBufferUploader::UploadBufferData( void* pData, uint32 u32NumElements, uint16 u16Stride, uint32 uGLbufferType, uint32 uGLstorageHint )
{
	//GLResourcePathManager* pResPathMgr = NULL;

	//if( uGLbufferType == GL_ARRAY_BUFFER ) 
	//	pResPathMgr = &GLVBOpathManager::GetInstance(); //VBO

	//else 
	//	pResPathMgr = &GLIBOpathManager::GetInstance(); //IBO

	//if( pszMeshIdentifier != NULL )
	//{
	//	if( pResPathMgr->HasResource( *pszMeshIdentifier ) )
	//		return pResPathMgr->GetResource( *pszMeshIdentifier );
	//}

	uint32 uBuffer;

	glGenBuffers( 1, &uBuffer ); 

	glBindBuffer( uGLbufferType, uBuffer );
	glBufferData( uGLbufferType, u16Stride * u32NumElements, pData, uGLstorageHint );
	glBindBuffer( uGLbufferType, 0 );


	/*if( pszMeshIdentifier != NULL )
		pResPathMgr->AddResource( *pszMeshIdentifier, uBuffer );*/

	return uBuffer;
}