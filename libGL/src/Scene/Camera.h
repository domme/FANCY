#ifndef CAMERA_H
#define CAMERA_H

#include "../includes.h"
#include "BoundingSphere.h"
#include "AABoundingBox.h"

class DLLEXPORT Camera
{
	friend class CameraController;

public:
	Camera();
	virtual ~Camera();

	const glm::mat4&		GetView() const						{ return m_matView; }
	const glm::mat4&		GetProjection() const				{ return m_matProjection; }
	const glm::mat4			GetViewProjection() const			{ return m_matViewProj; }
	glm::mat4 const			GetViewInvT() const					{ return glm::transpose( glm::inverse( m_matView ) ); }
	glm::mat4 const			GetViewInv() const					{ return glm::inverse( m_matView ); }
	glm::mat4 const			GetViewInvInvT() const				{ return glm::inverse( GetViewInvT() ); }
	glm::mat4 const			GetViewT() const					{ return glm::transpose( m_matView ); }
	void					SetFarPlane( float fFar )			{ m_fFar = fFar; }
	float					GetFarPlane() const					{ return m_fFar; }
	float					GetNearPlane() const				{ return m_fNear; }
	float					GetFovRad() const					{ return glm::radians( m_fFovDeg ); }
	float					GetFovDeg() const					{ return m_fFovDeg; }
	float					GetAspectRatio() const				{ return m_fWidth / m_fHeight; }
	
	glm::vec3				GetPosition() const;
	glm::vec3				GetSide() const;
	glm::vec3				GetForward() const;
	glm::vec3				GetUp() const;
	void					SetPosition( const glm::vec3& v3Pos );
	void					MoveForward( float fSpeed );
	void					MoveSideways( float fSpeed );
	void					SetView( const glm::mat4& rNewMatView );
	bool					IsVisible( const BoundingSphere& rSphereWS );
	void					SetProjectionPersp( float yFov_deg, float fWidth, float fHeight, float fNear, float fFar );
	void					SetProjectionOrtho( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar ); 
	AABoundingBox			getWSAABB();
	std::vector<glm::vec3>	getWSfrustumCorners();
	void					RotateFromMouseMove( float dx, float dy );
	void					SetOrientation( const glm::vec3& v3Side, const glm::vec3& v3Up, const glm::vec3& v3Forward );
		
private:
	enum EFrusumPlane
	{
		PLANE_LEFT = 0,
		PLANE_RIGHT,
		PLANE_BOTTOM,
		PLANE_TOP,
		PLANE_NEAR,
		PLANE_FAR
	};

	glm::mat4	m_matView;
	glm::mat4	m_matViewInverse;
	glm::mat4	m_matProjection;
	glm::mat4	m_matViewProj;
	
	float		m_fFovDeg;
	float		m_fFar;
	float		m_fNear;
	float		m_fMovementSpeed;
	glm::vec4	m_v4FrustumPlanesVS[ 6 ];
	float		m_fFocalLength;
	bool		m_bIsOrtho;
	float		m_fWidth;
	float		m_fHeight;

	void		updateFrustumPlanes();
	void		paramsChanged();
	void		rotateViewQuat( const float angle, const glm::vec3 v3Axis );
	

};

#endif