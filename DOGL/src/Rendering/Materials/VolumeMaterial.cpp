#include "VolumeMaterial.h"
#include "../UniformUtil.h"

GLuint VolumeMaterial::m_uRaystartTexture;

VolumeMaterial::VolumeMaterial() : Material() , m_uVolumeTexture( GLUINT_HANDLE_INVALID ), m_uTransferFunctionTexture( GLUINT_HANDLE_INVALID ), m_fSamplingRate( 2.0f ), m_fWindowValue( 0.2f ), m_uShadowTexture( GLUINT_HANDLE_INVALID ), m_bUseShadows( true ) 
{
	
}

VolumeMaterial::VolumeMaterial(  VolumeMaterial& other  ) : Material( other )
{
	m_uVolumeTexture = other.m_uVolumeTexture;
	m_uTransferFunctionTexture = other.m_uTransferFunctionTexture;
}


VolumeMaterial::~VolumeMaterial()
{
	Material::~Material();
}

void VolumeMaterial::UpdateUniform( IUniform* pUniform ) const
{
	using namespace ShaderSemantics;
	
	Material::UpdateUniform( pUniform );

	switch( pUniform->GetSemantic() )
	{
		case SAMPLING_RATE:
		{
			UniformUtil::UpdateUniform( pUniform, m_fSamplingRate );
		} break;

		case VOLUME_WINDOW_VALUE:
		{
			UniformUtil::UpdateUniform( pUniform, m_fWindowValue );
		} break;
		
		case VOLMAT_USE_SHADOWS:
		{
			UniformUtil::UpdateUniform( pUniform, m_bUseShadows );
		} break;
	}
}



