#include "Material.h"
#include "../../Engine.h"
#include "../Shader.h"

#include "../Uniform.h"
#include "../UniformUtil.h"


#include <sstream>

Material::Material() : 
m_pDeferredShader( NULL ),
	m_pForwardShader( NULL ),
	m_fOpacity( 0.0f ),
	m_v3AmbientReflectivity( 1.0f, 1.0f, 1.0f ),
	m_v3DiffuseReflectivity( 1.0f, 1.0f, 1.0f ),
	m_v3Color( 1.0f, 1.0f, 1.0f ),
	m_v3SpecularColor( 0.0f, 0.0f, 0.0f ),
	m_fGlossiness( 0.0f ),
	m_fSpecularExponent( 1.0f )
{

}

Material::Material(  Material& other  ) :
m_pDeferredShader( NULL ),
	m_pForwardShader( NULL ),
	m_fOpacity( 0.0f ),
	m_v3AmbientReflectivity( 1.0f, 1.0f, 1.0f ),
	m_v3DiffuseReflectivity( 1.0f, 1.0f, 1.0f ),
	m_v3Color( 1.0f, 1.0f, 1.0f ),
	m_v3SpecularColor( 0.0f, 0.0f, 0.0f ),
	m_fGlossiness( 0.0f ),
	m_fSpecularExponent( 1.0f )
{
	if( other.m_pDeferredShader )
		m_pDeferredShader = new Shader( *other.m_pDeferredShader );

	if( other.m_pForwardShader )
		m_pForwardShader = new Shader( *other.m_pForwardShader );

	m_fOpacity = other.m_fOpacity;
	m_v3AmbientReflectivity = other.m_v3AmbientReflectivity;
	m_v3DiffuseReflectivity = other.m_v3DiffuseReflectivity;
	m_v3Color = other.m_v3Color;
	m_v3SpecularColor = other.m_v3SpecularColor;
	m_fSpecularExponent = other.m_fSpecularExponent;
	m_fGlossiness = other.m_fGlossiness;
}


Material::~Material()
{
	SAFE_DELETE( m_pDeferredShader );
	SAFE_DELETE( m_pForwardShader );
}

void Material::assignAttributeSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic )
{
	IUniform* pUniform = pShader->GetAttributeByName( szName );

	if( pUniform )
		pUniform->SetSemantic( eSemantic );
}

void Material::assignUniformSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic )
{
	IUniform* pUniform = pShader->GetUniformByName( szName );

	if( pUniform )
		pUniform->SetSemantic( eSemantic );
}

void Material::assignUniformSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic, int index )
{
	IUniform* pUniform = pShader->GetUniformByName( szName );

	if( pUniform )
	{
		pUniform->SetSemantic( eSemantic );
		pUniform->SetIndex( index );
	}
}


void Material::ValidateMaterial() const
{
	bool bValid = validate();

	if( !bValid )
		LOG( "WARNING: Not all material-Textures are valid!" );
}


void Material::UpdateUniform( IUniform* pUniform ) const
{
	using namespace ShaderSemantics;

	Semantic eSemantic = pUniform->GetSemantic();

	switch( eSemantic )
	{
	case MATERIALCOLOR:
		{
			UniformUtil::UpdateUniform( pUniform, m_v3Color );
		} break;

	case AMBIENTREFLECTIVITY:
		{
			UniformUtil::UpdateUniform( pUniform, m_v3AmbientReflectivity );
		} break;

	case DIFFUSEREFLECTIVITY:
		{
			UniformUtil::UpdateUniform( pUniform, m_v3DiffuseReflectivity );
		} break;

	case SPECULARCOLOR:
		{
			UniformUtil::UpdateUniform( pUniform, m_v3SpecularColor );
		} break;

	case SPECULAREXPONENT:
		{
			UniformUtil::UpdateUniform( pUniform, m_fSpecularExponent );
		} break;

	case GLOSSINESS:
		{
			UniformUtil::UpdateUniform( pUniform, m_fGlossiness );
		} break;

	case TEXTURE:
		{
			UniformUtil::UpdateUniform( pUniform, glm::ivec2( pUniform->GetIndex(), GetTextureAtIndex( pUniform->GetIndex() ) ) );
		} break;

	case TEXTURE_1D:
		{
			UniformUtil::UpdateUniform( pUniform, glm::ivec2( pUniform->GetIndex(), GetTextureAtIndex( pUniform->GetIndex() ) ) );
		} break;

	case TEXTURE_3D:
		{
			UniformUtil::UpdateUniform( pUniform, glm::ivec2( pUniform->GetIndex(), GetTextureAtIndex( pUniform->GetIndex() ) ) );
		} break;

	case TEXTURE_CUBE:
		{
			UniformUtil::UpdateUniform( pUniform, glm::ivec2( pUniform->GetIndex(), GetTextureAtIndex( pUniform->GetIndex() ) ) );
		} break;

	case TEXTURE_SIZE:
		{
			UniformUtil::UpdateUniform( pUniform, GetTextureSizeAtIndex( pUniform->GetIndex() ) );
		} break;
	}
}



