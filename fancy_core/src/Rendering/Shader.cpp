#include "../includes.h"

#include "Shader.h"
#include "../IO/FileReader.h"
#include <iostream>
#include "Materials/Material.h"
#include "../Geometry/Mesh.h"
#include "../IO/GLSLpreprocessor.h"
#include "../IO/PathService.h"
#include "../IO/ShaderLoader.h"
#include "Uniform.h"

#include "Managers/GLResourceManager.h"

#include "Shader.h"

#define BUFSIZE 512


Shader::Shader() : m_uShaderProgram( GLUINT_HANDLE_INVALID )
{
	//STATIC_ASSERT( ARRAY_LENGTH( sShaderSemantics ) == ShaderSemantics::NUM );
}

Shader::Shader( Shader& other ) : m_uShaderProgram( GLUINT_HANDLE_INVALID )
{
	
	deleteResources();
	m_uShaderProgram = other.m_uShaderProgram;

	for( uint i = 0; i < other.m_vAttributes.size(); ++i )
	{
		IUniform* pAttributeClone = other.m_vAttributes[ i ]->Clone();
		pAttributeClone->SetObserver( this );
		m_vAttributes.push_back( pAttributeClone );
		m_mapActiveAttributes[ pAttributeClone->GetName() ] = pAttributeClone;
	}

	for( uint i = 0; i < other.m_vCachedUniforms.size(); ++i )
	{
		IUniform* pUniformClone = other.m_vCachedUniforms[ i ]->Clone();
		pUniformClone->SetObserver( this );
		pUniformClone->SetSemantic( pUniformClone->GetSemantic() ); //Has to be done in order to register this uniform in case it has a global semantic
		m_vCachedUniforms.push_back( pUniformClone );
		m_mapActiveUniforms[ pUniformClone->GetName() ] = pUniformClone;
	} 

	GLProgramResourceManager::GetInstance().AddResource( m_uShaderProgram );
}

Shader::~Shader()
{
	GLProgramResourceManager::GetInstance().HandleDelete( m_uShaderProgram );

	deleteResources();
}

void Shader::deleteResources()
{
	for( uint i = 0; i < m_vAttributes.size(); ++i )
	{
		delete m_vAttributes[ i ];
	}

	for( uint i = 0; i < m_vCachedUniforms.size(); ++i )
	{
		delete m_vCachedUniforms[ i ];
	} 

	m_vAttributes.clear();
	m_vCachedUniforms.clear();
	m_mapActiveUniforms.clear();
	m_mapActiveAttributes.clear();
}

void Shader::LoadShader( const std::string& szVertexShaderPath, const std::string& szFragmentShaderPath )
{
	if( m_uShaderProgram != GLUINT_HANDLE_INVALID )
	{
		GLProgramResourceManager::GetInstance().HandleDelete( m_uShaderProgram );
		deleteResources();
	}

	bool bSuccess = false;
	uint32 uProgram = ShaderLoader::GetInstance().LoadShaderProgram( szVertexShaderPath, szFragmentShaderPath, &bSuccess );

	if( !bSuccess )
	{
		LOG( std::string( "ERROR: Shader failed to load: " ) + szVertexShaderPath + " " + szFragmentShaderPath );
		return;
	}

	GLProgramResourceManager::GetInstance().AddResource( uProgram );
	
	m_uShaderProgram = uProgram;


	//Now build active uniform-map and active attribute-map
	int32 iNumActiveUniforms = 0;
	int32 iNumActiveAttributes = 0;
	glGetProgramiv( m_uShaderProgram, GL_ACTIVE_UNIFORMS, &iNumActiveUniforms );
	glGetProgramiv( m_uShaderProgram, GL_ACTIVE_ATTRIBUTES, &iNumActiveAttributes );

	for( int i = 0; i < iNumActiveUniforms; ++i )
	{
		GLchar szNameBuf[ BUFSIZE ];
		GLsizei iActualNameLength = 0;
		int32 iUniformSize = 0;
		uint32 eUniformType;
		int32 iUniformLoc = -1;
		IUniform* pUniform = NULL;

		glGetActiveUniform( m_uShaderProgram, i, BUFSIZE, &iActualNameLength, &iUniformSize, &eUniformType, szNameBuf );
		iUniformLoc = glGetUniformLocation( m_uShaderProgram, szNameBuf );
		
		String szName = String( szNameBuf );

		pUniform = createUniformFromDescription( eUniformType, szName, iUniformLoc );
		pUniform->SetObserver( this );

		if( m_mapActiveUniforms.find( szName ) != m_mapActiveUniforms.end() )
			LOG( "WARNING!! Uniform " + szName + " already in Map!" );

		m_mapActiveUniforms[ szName ] = pUniform;
		m_vCachedUniforms.push_back( pUniform );
	}

	for( int i = 0; i < iNumActiveAttributes; ++i )
	{
		GLchar szNameBuf[ BUFSIZE ];
		GLsizei iActualNameLength = 0;
		int32 iAttributeSize = 0;
		uint32 eAttributeType;
		int32 iAttributeLoc = -1;
		IUniform* pAttribute = NULL;

		glGetActiveAttrib( m_uShaderProgram, i, BUFSIZE, &iActualNameLength, &iAttributeSize, &eAttributeType, szNameBuf );
		iAttributeLoc = glGetAttribLocation( m_uShaderProgram, szNameBuf );

		String szName = String( szNameBuf );

		pAttribute = createAttributeFromDescription( eAttributeType, szName, iAttributeLoc );

		if( m_mapActiveAttributes.find( szName ) != m_mapActiveAttributes.end() )
			LOG( "WARNING!! Attribute " + szName + " already in Map!" );

		m_mapActiveAttributes[ szName ] = pAttribute;
		m_vAttributes.push_back( pAttribute );
	}
	
}

IUniform* Shader::createAttributeFromDescription( const uint32& eType, const String& szName, int32 iGLlocation )
{
	IUniform* pUniform = NULL;

	switch( eType )
	{
		case GL_FLOAT:
			{
				UniformFloat* pUniformImpl = new UniformFloat();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_VEC2:
			{
				UniformVec2* pUniformImpl = new UniformVec2();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_VEC3:
			{
				UniformVec3* pUniformImpl = new UniformVec3();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_VEC4:
			{
				UniformVec4* pUniformImpl = new UniformVec4();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			} break;

		case GL_INT:
			{
				UniformInt* pUniformImpl = new UniformInt();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_MAT2:
			{
				UniformMat2* pUniformImpl = new UniformMat2();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_MAT3:
			{
				UniformMat3* pUniformImpl = new UniformMat3();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		case GL_FLOAT_MAT4:
			{
				UniformMat4* pUniformImpl = new UniformMat4();
				pUniformImpl->m_iglHandle = iGLlocation;
				pUniformImpl->m_szName = szName;

				pUniform = pUniformImpl;
			}
			break;

		default:
			{
				LOG( "ERROR! No appropriate Attribute-type implemented for Attribute " + szName );
				return NULL;
			}
			break;
	} //end switch


	return pUniform;
}

IUniform* Shader::createUniformFromDescription( const uint32& eType, const String& szName, int32 iGLlocation )
{
	IUniform* pUniform = NULL;

	switch( eType )
	{
	case GL_FLOAT:
		{
			UniformFloat* pUniformImpl = new UniformFloat();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_VEC2:
		{
			UniformVec2* pUniformImpl = new UniformVec2();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_VEC3:
		{
			UniformVec3* pUniformImpl = new UniformVec3();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_VEC4:
		{
			UniformVec4* pUniformImpl = new UniformVec4();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		} break;

	case GL_INT:
		{
			UniformInt* pUniformImpl = new UniformInt();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_MAT2:
		{
			UniformMat2* pUniformImpl = new UniformMat2();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_MAT3:
		{
			UniformMat3* pUniformImpl = new UniformMat3();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_FLOAT_MAT4:
		{
			UniformMat4* pUniformImpl = new UniformMat4();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_SAMPLER_1D:
	case GL_SAMPLER_1D_SHADOW:
		{
			UniformTexture1D* pUniformImpl = new UniformTexture1D();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		} break;
		
	case GL_SAMPLER_2D:
	case GL_SAMPLER_2D_SHADOW:
		{
			UniformTexture2D* pUniformImpl = new UniformTexture2D();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		} break;

	case GL_SAMPLER_3D:
		{
			UniformTexture3D* pUniformImpl = new UniformTexture3D();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		} break;

	case GL_SAMPLER_CUBE:
#ifdef __WINDOWS
	case GL_SAMPLER_CUBE_SHADOW:
#endif
		{
			UniformTextureCube* pUniformImpl = new UniformTextureCube();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	case GL_BOOL:
		{
			UniformBool* pUniformImpl = new UniformBool();
			pUniformImpl->m_iglHandle = iGLlocation;
			pUniformImpl->m_szName = szName;

			pUniform = pUniformImpl;
		}
		break;

	default:
		{
			LOG( "ERROR! No appropriate Uniform-type implemented for Uniform " + szName );
			return NULL;
		}
		break;
	
	}

	
	return pUniform;
}

IUniform* Shader::GetAttributeByName( const String& szName )
{
	UniformMapType::iterator iter = m_mapActiveAttributes.find( szName );

	if( iter != m_mapActiveAttributes.end() )
		return iter->second;

	LOG( "WARNING! Attribute with name " + szName + " not in Shader's attribute-map!" );
	return NULL;
}


IUniform* Shader::GetUniformByName( const String& szName )
{
	UniformMapType::iterator iter = m_mapActiveUniforms.find( szName );

	if( iter != m_mapActiveUniforms.end() )
		return iter->second;

	LOG( "WARNING! Uniform with name " + szName + " not in Shader's uniform-map!" );
	return NULL;
}

void Shader::UniformChanged( IUniform* pUniform )
{
	UniformVectorType::iterator iter = std::find( m_vDirtyUniforms.begin(), m_vDirtyUniforms.end(), pUniform );

	if( iter != m_vDirtyUniforms.end() )
		return;

	m_vDirtyUniforms.push_back( pUniform );
}

/// This function may only be called after ApplyShader() 
void Shader::CleanUniforms()
{
	for( uint i = 0; i < m_vDirtyUniforms.size(); ++i )
	{
		m_vDirtyUniforms[ i ]->Upload();
	}

	m_vDirtyUniforms.clear();
}



 //bool Shader::AddAtribute( const String& szName, ShaderSemantics::Semantic semantic, int iNumber /* = 0 */ )
 //{
 //	SShaderSemantic sem( szName, semantic );
 //	sem.uGLlocation = GetVertexAttribLocation( szName.c_str() );
 //	sem.iN = iNumber;
 //	m_vSemantics.push_back( sem );
 //	return true;
 //}
 //
 //bool Shader::AddUniform( const String& szName, ShaderSemantics::Semantic semantic, int iNumber /* = 0 */ )
 //{
 //	SShaderSemantic sem( szName, semantic );
 //	sem.uGLlocation = GetUniformLocation( szName.c_str() );
 //	sem.iN = iNumber;
 //	m_vSemantics.push_back( sem );
 //	return true;
 //} 


uint Shader::GetVertexAttribLocation( const char* szVertexAttrname )
{
	return glGetAttribLocation( m_uShaderProgram, szVertexAttrname );
}

uint Shader::GetUniformLocation( const char* szUniformLoc )
{
	return glGetUniformLocation( m_uShaderProgram, szUniformLoc );
}


void Shader::ApplyShader() const
{
	glUseProgram( m_uShaderProgram );
}
