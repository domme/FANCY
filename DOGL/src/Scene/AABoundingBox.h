#ifndef AABOUNDINGBOX_H
#define AABOUNDINGBOX_H

#include "../includes.h"

class DLLEXPORT AABoundingBox
{
	

public:
	static AABoundingBox FromPoints( const glm::vec3* v3Points, int iPointCount );
	AABoundingBox();
	AABoundingBox( const glm::vec3& v3MinPos, const glm::vec3& v3MaxPos );
	~AABoundingBox();

	bool intersects( const glm::vec3& v3Point ) const;
	bool intersects( const AABoundingBox& box ) const;
	bool contains( const AABoundingBox& box ) const;

	AABoundingBox combine( const AABoundingBox& rBox );

	glm::vec3 getMidPoint() const;

	void getPoints( glm::vec3* v3Corners ) const
	{
		v3Corners[ 0 ] = m_v3MinPos;
		v3Corners[ 1 ] = glm::vec3( m_v3MaxPos.x, m_v3MinPos.y, m_v3MinPos.z );
		v3Corners[ 2 ] = glm::vec3( m_v3MaxPos.x, m_v3MinPos.y, m_v3MaxPos.z );
		v3Corners[ 3 ] = glm::vec3( m_v3MinPos.x, m_v3MinPos.y, m_v3MaxPos.z );
		v3Corners[ 4 ] = glm::vec3( m_v3MinPos.x, m_v3MaxPos.y, m_v3MinPos.z );
		v3Corners[ 5 ] = glm::vec3( m_v3MaxPos.x, m_v3MaxPos.y, m_v3MinPos.z );
		v3Corners[ 6 ] = m_v3MaxPos;
		v3Corners[ 7 ] = glm::vec3( m_v3MinPos.x, m_v3MaxPos.y, m_v3MaxPos.z );
	}

	AABoundingBox operator* ( const glm::mat4& rMat )
	{
		glm::vec3 v3Corners[ 8 ];

		getPoints( v3Corners );

		for( int i = 0; i < 8; ++i )
		{
			glm::vec4 v4Temp = rMat * glm::vec4( v3Corners[ i ], 1.0f );
			v3Corners[ i ] = glm::vec3( v4Temp.x, v4Temp.y, v4Temp.z );
		}

		return AABoundingBox::FromPoints( v3Corners, 8 );
	}

	bool operator== ( const AABoundingBox& rOtherBox )
	{
		float fBias = 0.01f;

		return	abs( m_v3MaxPos.x - rOtherBox.m_v3MaxPos.x ) < fBias &&
				abs( m_v3MaxPos.y - rOtherBox.m_v3MaxPos.y ) < fBias &&
				abs( m_v3MaxPos.z - rOtherBox.m_v3MaxPos.z ) < fBias &&
				abs( m_v3MinPos.x - rOtherBox.m_v3MinPos.x ) < fBias &&
				abs( m_v3MinPos.y - rOtherBox.m_v3MinPos.y ) < fBias &&
				abs( m_v3MinPos.z - rOtherBox.m_v3MinPos.z ) < fBias;
			   
	}

	glm::vec3 m_v3MinPos;
	glm::vec3 m_v3MaxPos;

private:



};

#endif