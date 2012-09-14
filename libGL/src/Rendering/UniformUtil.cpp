#include "UniformUtil.h"



void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::mat4& value )
{
	if( !pIUniform )
		return;

	UniformMat4* pUniformMat4 = dynamic_cast<UniformMat4*>( pIUniform );
	if( pUniformMat4 )
		pUniformMat4->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::mat3& value ) 
{
	if( !pIUniform )
		return;

	UniformMat3* pUniform = dynamic_cast<UniformMat3*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::vec4& value )
{
	if( !pIUniform )
		return;

	UniformVec4* pUniform = dynamic_cast<UniformVec4*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::vec3& value )
{
	if( !pIUniform )
		return;

	UniformVec3* pUniform = dynamic_cast<UniformVec3*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::vec2& value )
{
	if( !pIUniform )
		return;

	UniformVec2* pUniform = dynamic_cast<UniformVec2*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, float value )
{
	if( !pIUniform )
		return;

	UniformFloat* pUniform = dynamic_cast<UniformFloat*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, int value )
{
	if( !pIUniform )
		return;

	UniformInt* pUniform = dynamic_cast<UniformInt*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}

void UniformUtil::UpdateUniform( IUniform* pIUniform, bool value )
{
	if( !pIUniform )
		return;

	UniformBool* pUniform = dynamic_cast<UniformBool*>( pIUniform );
	if( pUniform )
		pUniform->SetValue( value );
}


void UniformUtil::UpdateUniform( IUniform* pIUniform, const glm::ivec2& value )
{
	if( !pIUniform )
		return;

	{
		UniformTexture2D* pUniform = dynamic_cast<UniformTexture2D*>( pIUniform );
		if( pUniform )
		{
			pUniform->SetValue( value );
			return;
		}
	}

	{
		UniformTexture3D* pUniform = dynamic_cast<UniformTexture3D*>( pIUniform );
		if( pUniform )
		{
			pUniform->SetValue( value );
			return;
		}
	}

	{
		UniformTextureCube* pUniform = dynamic_cast<UniformTextureCube*>( pIUniform );
		if( pUniform )
		{
			pUniform->SetValue( value );
			return;
		}
	}

	{
		UniformTexture1D* pUniform = dynamic_cast<UniformTexture1D*>( pIUniform );
		if( pUniform )
		{
			pUniform->SetValue( value );
			return;
		}
	}
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::mat4& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::mat3& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec4& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec3& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec2& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );

}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, float value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, int value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, bool value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}

void UniformUtil::UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::ivec2& value )
{
	for( uint i = 0; i < rvUniforms.size(); ++i )
		UpdateUniform( rvUniforms[ i ], value );
}