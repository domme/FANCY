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

AABoundingBox AABoundingBox::intersectionBox( const AABoundingBox& rBox )
{
	//NOTE: this function does not work properly and still has to be de-bugged. Use bounding-spheres instead in the meantime.

	AABoundingBox returnBox ( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ) );
	if( !intersects( rBox ) )
	{
		return returnBox;
	}

	if( contains( rBox ) )
	{
		return rBox;
	}

	if( (*this) == rBox )
	{
		return rBox;
	}
	
	glm::vec3 v3OtherPoints[ 8 ];
	glm::vec3 v3ThisPoints[ 8 ];
	getPoints( v3ThisPoints );
	rBox.getPoints( v3OtherPoints );

	std::vector<glm::vec3> vOtherPointsInThis;
	std::vector<glm::vec3> vThisPointsInOther;

	//Find all points that intersect the other bounding box
	for( int i = 0; i < 8; ++i )
	{
		if( this->intersects( v3OtherPoints[ i ] ) )
		{
			vOtherPointsInThis.push_back( v3OtherPoints[ i ] );
		}

		if( rBox.intersects( v3ThisPoints[ i ] ) )
		{
			vThisPointsInOther.push_back( v3ThisPoints[ i ] );
		}
	}

	//From only one of these points from each of the boxes a valid AABB can be constructed
	if( vOtherPointsInThis.size() > 0 && vThisPointsInOther.size() > 0 )
	{
		glm::vec3 v3Points[] = { vOtherPointsInThis[0], vThisPointsInOther[0] };
		returnBox = AABoundingBox::FromPoints( v3Points, 2 );
	}

	//None of the boxes' corners lie in one another
	else if( vOtherPointsInThis.size() == 0 && vThisPointsInOther.size() == 0 )
	{
		glm::vec3 v3Points[ 2 ];
		v3Points[ 0 ] = m_v3MinPos;
		v3Points[ 1 ] = m_v3MaxPos;

		if( m_v3MinPos.x < rBox.m_v3MinPos.x && m_v3MaxPos.x > rBox.m_v3MaxPos.x )
		{
			v3Points[ 0 ].x = rBox.m_v3MinPos.x;
			v3Points[ 1 ].x = rBox.m_v3MaxPos.x;
		}

		else if( m_v3MinPos.y < rBox.m_v3MinPos.y && m_v3MaxPos.y > rBox.m_v3MaxPos.y )
		{
			v3Points[ 0 ].y = rBox.m_v3MinPos.y;
			v3Points[ 1 ].y = rBox.m_v3MaxPos.y;
		}

		else if( m_v3MinPos.z < rBox.m_v3MinPos.z && m_v3MaxPos.z > rBox.m_v3MaxPos.z )
		{
			v3Points[ 0 ].z = rBox.m_v3MinPos.z;
			v3Points[ 1 ].z = rBox.m_v3MaxPos.z;
		}

		returnBox = AABoundingBox::FromPoints( v3Points, 2 );
	}

	//one of the boxes does not have any corner intersecting the other
	else
	{
		const AABoundingBox* pSmallBox;
		const AABoundingBox* pBigBox;
		AABoundingBox otherBox = rBox;
		std::vector<glm::vec3>* pvSmallInBig;
		//This sticks into other but other not in this
		if( !vOtherPointsInThis.size() > 0 && vThisPointsInOther.size() > 1 )
		{
			pSmallBox = this;
			pBigBox = &otherBox;
			pvSmallInBig = &vThisPointsInOther;
		}

		else
		{
			pSmallBox = &otherBox;
			pBigBox = this;
			pvSmallInBig = &vOtherPointsInThis;
		}
		
		//take the first two Intersecting points from this in other
		glm::vec3 v3Points[] = { (*pvSmallInBig)[ 0 ], (*pvSmallInBig)[ 1 ] };

		//find the side from wich the points stick in
		//Same x-coordinate--> small box comes from right or left
		if( abs( v3Points[ 0 ].x - v3Points[ 1 ].x ) < 0.001f ) 
		{
			if( pSmallBox->getMidPoint().x < pBigBox->getMidPoint().x ) //from left
			{
				v3Points[ 0 ].x = pBigBox->m_v3MinPos.x;
			}

			else if( pSmallBox->getMidPoint().x > pBigBox->getMidPoint().x ) //from right
			{
				v3Points[ 0 ].x = pBigBox->m_v3MaxPos.x;
			}
		}


		//Same y-Coordinate--> small box comes from above or below
		else if( abs( v3Points[ 0 ].y - v3Points[ 1 ].y ) < 0.001f ) 
		{
			if( pSmallBox->getMidPoint().y < pBigBox->getMidPoint().y ) //from below
			{
				v3Points[ 0 ].y = pBigBox->m_v3MinPos.y;
			}

			else if( pSmallBox->getMidPoint().y > pBigBox->getMidPoint().y ) //from above
			{
				v3Points[ 0 ].y = pBigBox->m_v3MaxPos.y;
			}
		}

		//Same z-Coordinate--> small box comes from front or behind
		else if( abs( v3Points[ 0 ].z - v3Points[ 1 ].z ) < 0.001f )
		{
			if( pSmallBox->getMidPoint().z < pBigBox->getMidPoint().z ) //from behind
			{
				v3Points[ 0 ].z = pBigBox->m_v3MinPos.z;
			}

			else if( pSmallBox->getMidPoint().z > pBigBox->getMidPoint().z ) //from front
			{
				v3Points[ 0 ].z = pBigBox->m_v3MaxPos.z;
			}
		}

		returnBox = AABoundingBox::FromPoints( v3Points, 2 );
	}

	return returnBox;
}

