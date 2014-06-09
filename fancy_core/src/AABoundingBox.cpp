#include "../includes.h"
#include "AABoundingBox.h"


AABoundingBox::AABoundingBox()
{
	m_v3MinPos = glm::vec3( 0,0,0 );
	m_v3MaxPos = glm::vec3( 1,1,1 );
	
}

AABoundingBox::~AABoundingBox()
{
	
}

glm::vec3 AABoundingBox::getMidPoint() const
{
	return ( m_v3MinPos + m_v3MaxPos ) / 2.0f;
}

AABoundingBox AABoundingBox::FromPoints( const glm::vec3* v3Points, int iPointCount )
{
	glm::vec3 v3Min ( 100000.0f, 100000.0f, 100000.0f );
	glm::vec3 v3Max ( -100000.0f, -100000.0f, -100000.0f );
	
	for( int i = 0; i < iPointCount; ++i )
	{
		const glm::vec3& rPoint = v3Points[ i ];

		v3Min.x = glm::min<float>( rPoint.x, v3Min.x );
		v3Min.y = glm::min<float>( rPoint.y, v3Min.y );
		v3Min.z = glm::min<float>( rPoint.z, v3Min.z );

		v3Max.x = glm::max<float>( rPoint.x, v3Max.x );
		v3Max.y = glm::max<float>( rPoint.y, v3Max.y );
		v3Max.z = glm::max<float>( rPoint.z, v3Max.z );
	}

	return AABoundingBox( v3Min, v3Max );
}

AABoundingBox::AABoundingBox( const glm::vec3& v3MinPos, const glm::vec3& v3MaxPos  )
{
	m_v3MinPos = v3MinPos;
	m_v3MaxPos = v3MaxPos;
}


/*inline*/ bool AABoundingBox::intersects( const glm::vec3& v3Point ) const
{
	return	( v3Point.x >= m_v3MinPos.x && v3Point.x <= m_v3MaxPos.x ) &&
			( v3Point.y >= m_v3MinPos.y && v3Point.y <= m_v3MaxPos.y ) &&
			( v3Point.z >= m_v3MinPos.z && v3Point.z <= m_v3MaxPos.z );
}

/*inline*/ bool AABoundingBox::intersects( const AABoundingBox& box ) const
{
	return	!	( ( m_v3MinPos.x > box.m_v3MaxPos.x ) || ( box.m_v3MinPos.x > m_v3MaxPos.x ) ) ||
				( ( m_v3MinPos.y > box.m_v3MaxPos.y ) || ( box.m_v3MinPos.y > m_v3MaxPos.y ) ) ||
				( ( m_v3MinPos.z > box.m_v3MaxPos.z ) || ( box.m_v3MinPos.z > m_v3MaxPos.z ) );
				
}

bool AABoundingBox::contains( const AABoundingBox& box ) const
{
	return m_v3MaxPos.x > box.m_v3MaxPos.x && m_v3MaxPos.y > box.m_v3MaxPos.y && m_v3MaxPos.z > box.m_v3MaxPos.z &&
		   m_v3MinPos.x < box.m_v3MinPos.x && m_v3MinPos.y < box.m_v3MinPos.y && m_v3MinPos.z < box.m_v3MinPos.z;
}

AABoundingBox AABoundingBox::combine( const AABoundingBox& rBox )
{
	glm::vec3 vPoints[] = {  m_v3MinPos, m_v3MaxPos, rBox.m_v3MinPos, rBox.m_v3MaxPos };
	return AABoundingBox::FromPoints( vPoints, 4 );
}

