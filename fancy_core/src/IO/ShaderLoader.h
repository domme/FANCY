#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include "../Includes.h"

class DLLEXPORT ShaderLoader
{
public: 
	~ShaderLoader();
	static ShaderLoader& GetInstance() { static ShaderLoader instance; return instance; }

	uint32 LoadShaderProgram( const String& szVertexShaderPath, const String& szFragmentShaderPath, bool* pbSuccess = NULL );

protected:
	ShaderLoader();

	void printShaderErrorLog( const uint32 uShaderHandler, const String& szShaderName );
	void printProgramErrorLog( const uint32 uProgramHandler, const String& szProgramName );
};

#endif