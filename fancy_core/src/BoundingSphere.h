#ifndef BOUNDINGSPHERE_H
#define BOUNDINGSPHERE_H

#include "../includes.h"
#include "AABoundingBox.h"

class DLLEXPORT BoundingSphere 
{
public:
	BoundingSphere();
	BoundingSphere( const glm::vec3& vCenter, float fR )
	{
		Init( vCenter, fR );
	}
	~BoundingSphere();
	
	//Operators
	const BoundingSphere operator*( const glm::mat4& matTransform ) const;


	const glm::vec3& getCenterPoint() const  { return m_vCenter; }
	void setCenterPoint( const glm::vec3& vCenter ) { m_vCenter = vCenter; }

	float getRadius() const { return m_fRadius; }
	void setRadius( float fR ) { m_fRadius = fR; }
	AABoundingBox getAABB() const;
	
	void Init( const glm::vec3& vCenter, float fR ) { setCenterPoint( vCenter ); setRadius( fR ); }
	
	bool contains( const BoundingSphere& rOtherSphere ) const
	{
		return glm::length( m_vCenter - rOtherSphere.m_vCenter ) + rOtherSphere.m_fRadius < m_fRadius;
	}

	BoundingSphere combine( const BoundingSphere& rOtherSphere );
	
private:
	glm::vec3	m_vCenter;
	float		m_fRadius;
	
};

#endif