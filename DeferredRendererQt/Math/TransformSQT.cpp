#include <Includes.h>
#include <Math/QuaternionService.h>

#include "TransformSQT.h"


TransformSQT::TransformSQT(): 
m_v3Translation( 0, 0, 0 ),
m_v3Scale( 1, 1, 1 ), 
m_quatRotation( 0, 1, 0, 0 )
{
	m_quatRotation = QuaternionService::CreateRotationQuaternion( 0.0f, glm::vec3( 0.0f, 1.0f, 0.0f ) );
}

TransformSQT::~TransformSQT()
{

}

TransformSQT::TransformSQT( const glm::mat4& rMat )
{
#ifdef MAT_ROW_MAJOR
	m_v3Translation = glm::vec3( rMat[ 3 ][ 0 ], rMat[ 3 ][ 1 ], rMat[ 3 ][ 2 ] );
#else
	m_v3Translation = glm::vec3( rMat[ 0 ][ 3 ], rMat[ 1 ][ 3 ], rMat[ 2 ][ 3 ] );
#endif

	m_v3Scale = glm::vec3( rMat[ 0 ][ 0 ], rMat[ 1 ][ 1 ], rMat[ 2 ][ 2 ] );
	m_quatRotation = glm::normalize( glm::quat_cast( rMat ) );
}



TransformSQT TransformSQT::concatenate( const TransformSQT& rOtherTransform ) const
{
	TransformSQT result;
	result.setData( *this );
	
	result.m_v3Translation	+= rOtherTransform.m_v3Translation;
	result.m_v3Scale		*= rOtherTransform.m_v3Scale;
	result.m_quatRotation	= glm::normalize( glm::cross( result.m_quatRotation, rOtherTransform.m_quatRotation ) );
	
	return result;
}

void TransformSQT::appendToMatrix( glm::mat4& rMat )
{
	rMat *= this->getAsMat4(); 
}

void TransformSQT::setData( const TransformSQT& rOtherTransform )
{
	m_v3Translation = rOtherTransform.m_v3Translation;
	m_v3Scale		= rOtherTransform.m_v3Scale;
	m_quatRotation	= rOtherTransform.m_quatRotation;
}

const glm::mat4 TransformSQT::getAsMat4() const
{
	glm::mat4 returnMat ( 1, 0, 0, 0,
						  0, 1, 0, 0,
						  0, 0, 1, 0,
						  0, 0, 0, 1 );
	
	returnMat *= glm::translate( m_v3Translation );
	returnMat *= glm::scale( m_v3Scale );
	returnMat *= glm::mat4_cast( m_quatRotation );
	
	return returnMat;
}





