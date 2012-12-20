#ifndef SHADER_H
#define SHADER_H

#include "../includes.h"
#include "ShaderSemantics.h"
#include "IUniformObserver.h"
#include "IUniform.h"

class Mesh;

/*
struct SShaderSemantic
{
	SShaderSemantic( const std::string& _szName, ShaderSemantics::Semantic _eSemantic ) 
	{
		szName = _szName;
		eSemantic = _eSemantic;
		iN = -1;
		uGLlocation = 0xFFFFFFFF;
	}

	std::string szName;
	ShaderSemantics::Semantic eSemantic;
	int iN;
	GLuint uGLlocation;
}; */

class DLLEXPORT Shader : public IUniformObserver
{
public:
	Shader();
	Shader( Shader& other );
	virtual ~Shader();
	static GLenum TexUnitFromIdx( uint uIndex ); 

	void LoadShader( const std::string& szVertexShaderPath, const std::string& szFragmentShaderPath );
	//bool AddAtribute( const String& szName, ShaderSemantics::Semantic semantic, int iNumber = 0 );
	//bool AddUniform( const String& szName, ShaderSemantics::Semantic semantic, int iNumber = 0 );
	IUniform* GetUniformByName( const String& szName );
	IUniform* GetAttributeByName( const String& szName ); 
	const std::vector<IUniform*>& GetUniforms() { return m_vCachedUniforms; }
	const std::vector<IUniform*>& GetAttributes() { return m_vAttributes; }

	uint GetVertexAttribLocation( const char* szVertexAttrname );
	uint GetUniformLocation( const char* szUniformLoc );

	void ApplyShader() const;

	virtual void UniformChanged( IUniform* pUniform );
	virtual void CleanUniforms();

	
protected:
	typedef std::map<String, IUniform*> UniformMapType;
	typedef std::vector<IUniform*> UniformVectorType;

	UniformMapType m_mapActiveUniforms;
	UniformMapType m_mapActiveAttributes; //TODO: Not sure if needed yet

	UniformVectorType m_vAttributes;
	UniformVectorType m_vCachedUniforms;
	UniformVectorType m_vDirtyUniforms;
	
	GLuint m_uShaderProgram;
	static IUniform* createUniformFromDescription( const GLenum& eType, const String& szName, GLint iGLlocation );
	static IUniform* createAttributeFromDescription( const GLenum& eType, const String& szName, GLint iGLlocation );
	void deleteResources();

	
};

#endif
