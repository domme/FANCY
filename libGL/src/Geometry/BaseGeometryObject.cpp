#include "BaseGeometryObject.h"


void BaseGeometryObject::initBoundingGeometry( const std::vector<glm::vec3>& vPositions )
{
	//Construct AABB
	assert( vPositions.size() > 0 );

	m_clAABB = AABoundingBox::FromPoints( &vPositions[ 0 ], vPositions.size() );

	glm::vec3 v3Center;
	v3Center.x = ( m_clAABB.m_v3MinPos.x + m_clAABB.m_v3MaxPos.x ) / 2.0f;
	v3Center.y = ( m_clAABB.m_v3MinPos.y + m_clAABB.m_v3MaxPos.y ) / 2.0f;
	v3Center.z = ( m_clAABB.m_v3MinPos.z + m_clAABB.m_v3MaxPos.z ) / 2.0f;

	float fR = 0.0f;
	//look for the greatest length between center point and vertex
	for( unsigned int i = 0; i < vPositions.size(); ++i )
	{
		const glm::vec3& currVec = vPositions[ i ];
		float fCurrR = glm::length( v3Center - currVec );
		if( fCurrR > fR )
		{
			fR = fCurrR;
		}
	}

	m_clBoundingSphere.Init( v3Center, fR );
}