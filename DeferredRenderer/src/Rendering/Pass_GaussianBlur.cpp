#include <Services/FBOservice.h>
#include <Rendering/GLRenderer.h>

#include "Pass_GaussianBlur.h"

Pass_GaussianBlur::Pass_GaussianBlur() :
m_uHeight( 512 ),
m_uWidth( 512 ),
m_pFSquad( NULL ),
m_eInternalFormat( GL_RGBA8 ),
m_eFormat( GL_RGBA ),
m_eDataType( GL_UNSIGNED_BYTE ),
m_uBlurStrength( 3 )
{
	m_pFSquad = &FullscreenQuad::getInstance();
	
	m_clBlurMat.Init();
}

Pass_GaussianBlur::~Pass_GaussianBlur()
{
	deleteTempFBOandTex();
}

void Pass_GaussianBlur::Init( uint uBlurStrength, uint uDestWidth, uint uDestHeight, GLenum eInternalFormat, GLenum eFormat, GLenum eDataType )
{
	m_uBlurStrength = uBlurStrength;
	m_uWidth = uDestWidth;
	m_uHeight = uDestHeight;
	m_eInternalFormat = eInternalFormat;
	m_eFormat = eFormat;
	m_eDataType = eDataType;

	createTempFBOandTex();
	createGaussAndOffsetTex();
}

void Pass_GaussianBlur::deleteTempFBOandTex()
{
	glDeleteTextures( 1, &m_uTempTex );
	glDeleteFramebuffers( 1, &m_uTempFBO );
}

void Pass_GaussianBlur::createTempFBOandTex()
{
	static bool bInit = false;

	if( bInit )
	{
		deleteTempFBOandTex();
	}

	bInit = true;

	glGenFramebuffers( 1, &m_uTempFBO );
	glGenTextures( 1, &m_uTempTex );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uTempFBO );
	glBindTexture( GL_TEXTURE_2D, m_uTempTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
	glTexImage2D( GL_TEXTURE_2D, 0, m_eInternalFormat, m_uWidth, m_uHeight, 0, m_eFormat, m_eDataType, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uTempTex, 0 );

	FBOservice::checkFBOErrors();

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Pass_GaussianBlur::deleteGaussAndOffsetTex()
{
	glDeleteTextures( 1, &m_uGaussTex );
	glDeleteTextures( 1, &m_uOffsetTex );
}

void fibonacci( std::vector<uint>& values, int iCurrIndex, int iEndIndex, std::vector<uint>& rFinalValues )
{
	std::vector<uint> newValues;
	newValues.push_back( 1 );

	for( int iValue = 0; iValue < iCurrIndex; ++iValue )
	{
		newValues.push_back( values[ iValue ] + values[ iValue + 1 ] );
	}
	
	newValues.push_back( 1 );
	++iCurrIndex;

	if( iCurrIndex == iEndIndex )
	{
		for( int i = 0; i < newValues.size(); ++i )
		{
			rFinalValues.push_back( newValues[ i ] );
		}

		return;
	}
		
	return fibonacci( newValues, iCurrIndex, iEndIndex, rFinalValues );
}



void Pass_GaussianBlur::createGaussAndOffsetTex()
{
	static bool bInit = false;

	if( bInit )
	{
		deleteGaussAndOffsetTex();
	}

	bInit = true;

	int uFibIndex = m_uBlurStrength * 2;
	
	uint64 iSum = 2;
	for( int i = 0; i < uFibIndex - 1; ++i )
	{
		iSum *= 2;
	}

	int iFibLength = uFibIndex + 1;
	int iMidIndex = m_uBlurStrength;

	std::vector<uint> vFibonacci;
	std::vector<uint> vStartValues;
	vStartValues.push_back( 1 );
	fibonacci( vStartValues, 0, uFibIndex, vFibonacci );

	//POW
	std::vector<float> vfGauss;
	for( uint i = iMidIndex; i < iFibLength; ++i )
	{
		vfGauss.push_back( (float) vFibonacci[ i ] / iSum );
	}

	std::vector<float> vfOffsets;
	uint k = 0;
	for( uint i = iMidIndex; i < iFibLength; ++i )
	{
		vfOffsets.push_back( (float) k++ );
	}

	std::vector<float> vfLinearGauss;
	std::vector<float> vfLinearOffsets;

	vfLinearGauss.push_back( vfGauss[ 0 ] );
	vfLinearOffsets.push_back( vfOffsets[ 0 ] );

	for( uint i = 1; i < vfGauss.size(); i += 2 )
	{
		float  fLinearGauss = 0.0f;
		float  fLinearOffset = 0.0f;
		
		if( i < vfGauss.size() - 1 )
		{
			fLinearGauss = vfGauss[ i ] + vfGauss[ i + 1 ];
			fLinearOffset = ( vfOffsets[ i ] * vfGauss[ i ] + vfOffsets[ i + 1 ] * vfGauss[ i + 1 ] ) / fLinearGauss;
		}

		else
		{
			fLinearGauss = vfGauss[ i ];
			fLinearOffset = vfOffsets[ i ];
		}
		
		vfLinearGauss.push_back( fLinearGauss );
		vfLinearOffsets.push_back( fLinearOffset );
	}
		
	glGenTextures( 1, &m_uGaussTex );
	glGenTextures( 1, &m_uOffsetTex );

	glBindTexture( GL_TEXTURE_1D, m_uGaussTex );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, vfLinearGauss.size(), 0, GL_RED, GL_FLOAT, &vfLinearGauss[ 0 ] );

	glBindTexture( GL_TEXTURE_1D, m_uOffsetTex );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, vfLinearOffsets.size(), 0, GL_RED, GL_FLOAT, &vfLinearOffsets[ 0 ] );

	m_clBlurMat.SetGaussTexture( m_uGaussTex );
	m_clBlurMat.SetOffsetTexture( m_uOffsetTex );

	m_clBlurMat.SetKernelSize( vfLinearGauss.size() );
}


void Pass_GaussianBlur::BlurTextureIntoFBO( GLuint uSrcTexture, GLuint uDestFBO, uint uDestWidth, uint uDestHeight, GLenum eInternalFormat, GLenum eFormat, GLenum eDatatype, GLRenderer* pRenderer, uint uBlurStrength /* = 3 */ )
{
	if( m_uWidth			!= uDestWidth ||
		m_uHeight			!= uDestHeight ||
		m_eInternalFormat	!= eInternalFormat ||
		m_eFormat			!= eFormat ||
		m_eDataType			!= eDatatype )
	{

		m_uWidth = uDestWidth;
		m_uHeight = uDestHeight;
		m_eInternalFormat = eInternalFormat;
		m_eFormat = eFormat;
		m_eDataType = eDatatype;

		createTempFBOandTex();
	}


	if( m_uBlurStrength != uBlurStrength )
	{
		m_uBlurStrength = uBlurStrength;

		createGaussAndOffsetTex();
	}


	//////////////////////////////////////////////////////////////////////////
	pRenderer->setViewport( 0, 0, m_uWidth, m_uHeight );
	
	//1st Pass: Blur Horizontal into TempTex
	glBindFramebuffer( GL_FRAMEBUFFER, m_uTempFBO );
	m_clBlurMat.SetInputTexture( uSrcTexture );
	m_clBlurMat.SetSamplingDirection( ESamplingDirection::DIRECTION_HORIZONTAL );
	m_pFSquad->RenderWithMaterial( &m_clBlurMat );

	//2nd Pass: Blur Vertical into Dest-FBO
	glBindFramebuffer( GL_FRAMEBUFFER, uDestFBO );
	m_clBlurMat.SetInputTexture( m_uTempTex );
	m_clBlurMat.SetSamplingDirection( ESamplingDirection::DIRECTION_VERTICAL );
	m_pFSquad->RenderWithMaterial( &m_clBlurMat );
}