#ifndef BASEGEOMETRY_H
#define BASEGEOMETRY_H

#include "../Includes.h"
#include "../Scene/BoundingSphere.h"
#include "../Scene/AABoundingBox.h"

class DLLEXPORT BaseGeometryObject
{
public:
	const BoundingSphere&	GetBoundingSphere() const { return m_clBoundingSphere; }
	const AABoundingBox&	GetAABB() const { return m_clAABB; }
	const String&			GetName() const { return m_szName; }

	void					SetName( const String& szName ) { m_szName = szName; }

protected:
	BoundingSphere			m_clBoundingSphere;
	AABoundingBox			m_clAABB;
	String					m_szName;

	void					initBoundingGeometry( const std::vector<glm::vec3>& vPositions );


};

#endif


