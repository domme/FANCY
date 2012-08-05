#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include "../Includes.h"

class ShaderLoader
{
public: 
	~ShaderLoader();
	static ShaderLoader& GetInstance() { static ShaderLoader instance; return instance; }

	GLuint LoadShaderProgram( const String& szVertexShaderPath, const String& szFragmentShaderPath, bool* pbSuccess = NULL );

protected:
	ShaderLoader();

	void printShaderErrorLog( const GLuint uShaderHandler, const String& szShaderName );
	void printProgramErrorLog( const GLuint uProgramHandler, const String& szProgramName );
};

#endif