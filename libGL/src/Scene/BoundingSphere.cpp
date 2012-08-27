#include "BoundingSphere.h"

BoundingSphere::BoundingSphere() : m_vCenter( 0.0f, 0.0f, 0.0f ), m_fRadius( 1.0f )
{
}

BoundingSphere::~BoundingSphere()
{

}

AABoundingBox BoundingSphere::getAABB() const
{
	AABoundingBox returnBox;
	returnBox.m_v3MinPos = glm::vec3( m_vCenter.x - m_fRadius, m_vCenter.y - m_fRadius, m_vCenter.z - m_fRadius );
	returnBox.m_v3MaxPos = glm::vec3( m_vCenter.x + m_fRadius, m_vCenter.y + m_fRadius, m_vCenter.z + m_fRadius );
	return returnBox;
}

BoundingSphere BoundingSphere::operator* ( const glm::mat4& matTransform ) const
{
	BoundingSphere resultSphere;
	glm::vec4 v4Center = matTransform * glm::vec4( m_vCenter, 1.0f );
	resultSphere.m_vCenter.x = v4Center.x;
	resultSphere.m_vCenter.y = v4Center.y;
	resultSphere.m_vCenter.z = v4Center.z;

	//get the scaling factor of this matrix - assuming a uniform scaling!
	float fScaleFact = matTransform[ 0 ][ 0 ];
	resultSphere.m_fRadius = m_fRadius * fScaleFact;
	return resultSphere;
}

BoundingSphere BoundingSphere::combine( const BoundingSphere& rOtherSphere )
{
	if( contains( rOtherSphere ) )
	{
		return (*this);
	}

	if( rOtherSphere.contains( (*this) ) )
	{
		return rOtherSphere;
	}

	glm::vec3 v3MidPoint = ( m_vCenter + rOtherSphere.m_vCenter ) / 2.0f;
	float fR = glm::max<float>( glm::length( v3MidPoint - m_vCenter ), glm::length( v3MidPoint - rOtherSphere.m_vCenter ) );
	fR += glm::max<float>( m_fRadius, rOtherSphere.m_fRadius );

	return BoundingSphere( v3MidPoint, fR );
}