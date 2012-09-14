#ifndef MODELLOADER_LOGSTREAM
#define MODELLOADER_LOGSTREAM

#include "../Includes.h"
#include <assimp/LogStream.h>

class ModelLoader_LogStream : public Assimp::LogStream
{
public:
	ModelLoader_LogStream() {}
	~ModelLoader_LogStream() {}

	void write( const char* message ) { LOG( String( message ) ); }
};

#endif