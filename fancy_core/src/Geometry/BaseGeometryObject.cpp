#include "BaseGeometryObject.h"


void BaseGeometryObject::initBoundingGeometry( const std::vector<glm::vec3>& vPositions )
{
	//Construct AABB
	assert( vPositions.size() > 0 );

	m_clAABB = AABoundingBox::FromPoints( &vPositions[ 0 ], vPositions.size() );

	glm::vec3 v3Center = ( m_clAABB.m_v3MinPos + m_clAABB.m_v3MaxPos ) / 2.0f;
	
	float fSphereRadius = glm::length( v3Center - m_clAABB.m_v3MaxPos );
	m_clBoundingSphere.Init( v3Center, fSphereRadius );
}