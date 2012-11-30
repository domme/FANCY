#ifndef CAMERA_H
#define CAMERA_H

#include "../includes.h"
#include "BoundingSphere.h"
#include "AABoundingBox.h"

class DLLEXPORT Camera
{
	friend class CameraController;

public:

	struct SFrustumPlane
	{
		float a;
		float b;
		float c;
		float d;
	};

	Camera();
	virtual ~Camera();

	void InitView( const glm::vec4& _v4Eye, const glm::vec4& _v4At, const glm::vec4& _v4UserUpDir );
	void InitView( const glm::vec3& _v3Eye, const glm::vec3& _v3At, const glm::vec3& _v3UserUpDir );
	void InitPerspectiveProjection( const float fFov, const float fRatio, const float fNear, const float fFar );
	void InitPerspectiveProjection( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar );
	void InitOrthogonalProjection( float fLeft, float fRight, float fBottom, float fTop, float fDummyFarPlane );
	void InitOrthogonalProjection( float l, float r, float b, float t, float n, float f );

	void MoveForward( float fSpeed );
	void MoveSideways( float fSpeed );

	const glm::mat4& GetView() const { return m_matView; }
	const glm::mat4& GetProjection() const { return m_matProjection; }
	const glm::mat4 GetViewProjection() const { return m_matViewProj; }
	const glm::vec3& getPosition() const { return m_v3Position; }
	glm::mat4 const GetViewInvT() const { return glm::transpose( glm::inverse( m_matView ) ); }
	glm::mat4 const GetViewInv() const { return glm::inverse( m_matView ); }
	glm::mat4 const GetViewInvInvT() const { return glm::inverse( GetViewInvT() ); }
	glm::mat4 const GetViewT() const { return glm::transpose( m_matView ); }

	void SetView( const glm::mat4& rNewMatView );
	void SetProjection( const glm::mat4& rNewMatProjection );
	void SetFarPlane( float fFar ) { m_fFar = fFar; }
	float getFarPlane() const { return m_fFar; }
	float getNearPlane() const { return m_fNear; }
	float getFovRad() const { return glm::radians( m_fFov ); }
	float getFovDeg() const { return m_fFov; }
	const SFrustumPlane* getFrustumPlanes() const { return m_FrustumPlanes; }
	AABoundingBox getWSAABB();
	std::vector<glm::vec3> getWSfrustumCorners();

	void Update( const uint elapsedTicksMS );

	bool IsVisible( const BoundingSphere& rSphereWS );

	void RotateFromMouseMove( float dx, float dy );
		
protected:

	glm::mat4 m_matView;
	glm::mat4 m_matViewInverse;
	glm::mat4 m_matProjection;
	glm::mat4 m_matViewProj;
	
	glm::vec3 m_v3View;
	glm::vec3 m_v3Position;
	glm::vec3 m_v3Up;
	glm::vec3 m_v3Side;

	float		m_fFov;

	float		m_fFar;
	float		m_fNear;
	float		m_fMovementSpeed;
	SFrustumPlane m_FrustumPlanes[ 6 ];

	void CheckOriantationMatrix( const uint elapsedTicksMS );
	void RecalculateMatrix();
	void UpdateViewProj() { m_matViewProj = m_matProjection * m_matView; }
	void UpdateFrustumPlanes();
	void recalculateMembers();
	
	void RotateViewQuat( const float angle, const glm::vec3 v3Axis );
};

#endif