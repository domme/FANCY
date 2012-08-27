#include "ShaderLoader.h"

#include "../Rendering/Managers/GLResourcePathManager.h"
#include "GLSLpreprocessor.h"
#include "FileReader.h"

ShaderLoader::ShaderLoader()
{

}

ShaderLoader::~ShaderLoader()
{

}

GLuint ShaderLoader::LoadShaderProgram( const String& szVertexShaderPath, const String& szFragmentShaderPath, bool* pbSuccess /* = NULL */ )
{
	GLShaderPathManager& rShaderPathMgr = GLShaderPathManager::GetInstance();
	
	String szProgramKey = szVertexShaderPath + szFragmentShaderPath;

	if( rShaderPathMgr.HasResource( szProgramKey ) )
	{
		if( pbSuccess )
			*pbSuccess = true;

		return rShaderPathMgr.GetResource( szProgramKey );
	}
		
	//Shader program not loaded yet!
	GLuint uVertexShader, uFragmentShader, uShaderProgram;

	String szVertexShaderSrc = "";
	String szFragmentShaderSrc = "";

	GLSLpreprocessor::getInstance().preprocessShader( szVertexShaderSrc, szVertexShaderPath );
	GLSLpreprocessor::getInstance().preprocessShader( szFragmentShaderSrc, szFragmentShaderPath );
	

	GLint iResult;
	//create the shader Objects and the program
	uVertexShader = glCreateShader( GL_VERTEX_SHADER );
	uFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	uShaderProgram = glCreateProgram();
	
	const char* szTemp =  szVertexShaderSrc.c_str(); 
	glShaderSource( uVertexShader, 1, &szTemp, NULL);
	glCompileShader( uVertexShader );
	glGetShaderiv( uVertexShader, GL_COMPILE_STATUS, &iResult );
	if( iResult != GL_TRUE )
	{
		printShaderErrorLog( uVertexShader, szVertexShaderPath );
		glDeleteShader( uVertexShader );
		glDeleteShader( uFragmentShader );
		glDeleteProgram( uShaderProgram );
		
		if( pbSuccess )
			*pbSuccess = false;
		
		return 0;
	}


	szTemp = szFragmentShaderSrc.c_str();
	glShaderSource( uFragmentShader, 1, &szTemp, NULL );
	glCompileShader( uFragmentShader );
	glGetShaderiv( uFragmentShader, GL_COMPILE_STATUS, &iResult ); 
	if( iResult != GL_TRUE )
	{
		printShaderErrorLog( uFragmentShader, szFragmentShaderPath );
		glDeleteShader( uVertexShader );
		glDeleteShader( uFragmentShader );
		glDeleteProgram( uShaderProgram );
		
		if( pbSuccess )
			*pbSuccess = false;

		return 0;
	}
	
	glAttachShader( uShaderProgram, uVertexShader );
	glAttachShader( uShaderProgram, uFragmentShader );

	glLinkProgram( uShaderProgram );

	glGetProgramiv( uShaderProgram, GL_LINK_STATUS, &iResult );
	if( iResult != GL_TRUE )
	{
		printProgramErrorLog( uShaderProgram, szProgramKey );
		glDeleteShader( uVertexShader );
		glDeleteShader( uFragmentShader );
		glDeleteProgram( uShaderProgram );
		
		if( pbSuccess )
			*pbSuccess = false;

		return 0;
	}
	
	//Flag the shader objects for deletion so that they are deleted once the program is deleted:
	glDeleteShader( uVertexShader );
	glDeleteShader( uFragmentShader );
	
	rShaderPathMgr.AddResource( szProgramKey, uShaderProgram );
	
	if( pbSuccess )
		*pbSuccess = true;

	return uShaderProgram;
}

void ShaderLoader::printShaderErrorLog( const GLuint uShaderHandler, const String& szShaderName )
{
	GLchar szInfo[ 2048 ];
	GLint  iLength = 0;
	glGetShaderInfoLog( uShaderHandler, 2048, &iLength, szInfo);

	LOG( "\n\n" + szShaderName + ":\n" + String( szInfo ) + "\n" );
}

void ShaderLoader::printProgramErrorLog( const GLuint uProgramHandler, const String& szProgramName )
{
	GLchar szInfo[ 2048 ];
	GLint  iLength = 0;
	glGetProgramInfoLog( uProgramHandler, 2048, &iLength, szInfo);

	LOG( "\n\n" + szProgramName + ":\n" + String( szInfo ) + "\n" );
}