#include  <math.h>
#include <stdio.h>

#include "../includes.h"
#include "Camera.h"
#include "../Rendering/GLRenderer.h"


Camera::Camera():
m_fMovementSpeed( 50.0f )
{

}

Camera::~Camera()
{

}


void Camera::InitView( const glm::vec4& _v4Eye, const glm::vec4& _v4At, const glm::vec4& _v4UserUpDir )
{
	glm::vec3 v3Eye( _v4Eye.x / _v4Eye.w, _v4Eye.y / _v4Eye.w, _v4Eye.z / _v4Eye.w );
	glm::vec3 v3At( _v4At.x / _v4At.w, _v4At.y / _v4At.w, _v4At.z / _v4At.w );
	v3At = v3At;
	
	glm::vec3 v3UserUp( _v4UserUpDir.x / _v4UserUpDir.w, _v4UserUpDir.y / _v4UserUpDir.w, _v4UserUpDir.z / _v4UserUpDir.w );
	v3UserUp = glm::normalize( v3UserUp );
	
	m_v3Position	= v3Eye;
	m_v3View		= glm::normalize( v3At - v3Eye );
	m_v3Up			= glm::normalize( v3UserUp ); 
	m_v3Side		= glm::normalize( glm::cross( m_v3View, m_v3Up ) );
		
    m_matView = glm::lookAt( v3Eye, v3At, v3UserUp ); 
}

void Camera::InitView( const glm::vec3& _v3Eye, const glm::vec3& _v3At, const glm::vec3& _v3UserUpDir )
{
	glm::vec3 v3At = _v3At;
	glm::vec3 v3UserUp = _v3UserUpDir;

	m_v3Position	= _v3Eye; 
	m_v3View		= glm::normalize( v3At - _v3Eye );
	m_v3Up			= glm::normalize( v3UserUp ); 
	m_v3Side		= glm::normalize( glm::cross( m_v3View, m_v3Up ) );
	
	
	m_matView = glm::lookAt( _v3Eye, v3At, v3UserUp );
}

void Camera::InitOrthogonalProjection( float fLeft, float fRight, float fBottom, float fTop, float fDummyFarPlane )
{
	m_matProjection = glm::ortho( fLeft, fRight, fBottom, fTop );
	m_fFar = fDummyFarPlane;

	recalculateMembers();
	Update(0);
}

void Camera::InitOrthogonalProjection( float l, float r, float b, float t, float n, float f )
{
	m_fFar = f;
	m_fNear = n;
	m_fFov = 60;

	f *= -1.0f;
	n *= -1.0f;

	glm::mat4 Result(0);
	Result[0][0] = 2.0f / (r - l);
	Result[1][1] = 2.0f / (t - b);
	Result[2][2] = 2.0f / (f - n);
	Result[3][0] = - (r + l) / (r - l);
	Result[3][1] = - (t + b) / (t - b);
	Result[3][2] = - (f + n) / (f - n);
	Result[3][3] = 1.0f;

	m_matProjection = Result;

	recalculateMembers();
	Update(0);
}


void Camera::InitPerspectiveProjection( const float fFov, const float fRatio, const float fNear, const float fFar )
{
	m_matProjection = glm::perspective( fFov, fRatio, fNear, fFar );
	m_fFar = fFar;
	m_fNear = fNear;
	m_fFov = fFov;
	recalculateMembers();
	Update(0);
}

void Camera::InitPerspectiveProjection( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar )
{
	m_matProjection = glm::frustum( fLeft, fRight, fBottom, fTop, fNear, fFar );
	m_fFar = fFar;
	m_fNear = fNear;
	m_fFov = ( abs( fRight ) + abs( fLeft ) ) / ( abs( fBottom ) + abs( fTop ) );

	recalculateMembers();
	Update(0);
}


void Camera::SetView( const glm::mat4& rNewMatView )
{
	m_matView = rNewMatView;
	recalculateMembers();
	Update(0);
}


void Camera::SetProjection( const glm::mat4& rNewMatProjection )
{
	m_matProjection = rNewMatProjection;
	UpdateViewProj();
	UpdateFrustumPlanes();
}

void Camera::recalculateMembers()
{
	glm::vec4 v4LocalUp = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );
	glm::vec4 v4LocalSide = glm::vec4( 1.0f, 0.0f, 0.0f, 0.0f );
	glm::vec4 v4LocalView = glm::vec4( 0.0f, 0.0f, -1.0f, 0.0f );
	glm::vec4 v4LocalPos = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );

	glm::mat4 m4ViewInv = GetViewInv();

	glm::vec4 v4TransformedUp = m4ViewInv * v4LocalUp;
	glm::vec4 v4TransformedSide = m4ViewInv * v4LocalSide;
	glm::vec4 v4TransformedView = m4ViewInv * v4LocalView;
	glm::vec4 v4TransformedPos = m4ViewInv * v4LocalPos;

	m_v3Position = glm::vec3( v4TransformedPos.x, v4TransformedPos.y, v4TransformedPos.z ) ;
	m_v3Side = glm::normalize( glm::vec3( v4TransformedSide.x, v4TransformedSide.y, v4TransformedSide.z ) );
	m_v3Up = glm::normalize( glm::vec3( v4TransformedUp.x, v4TransformedUp.y, v4TransformedUp.z ) );
	m_v3View = glm::normalize( glm::vec3( v4TransformedView.x, v4TransformedView.y, v4TransformedView.z ) );
}


void Camera::Update( const uint elapsedTicksMS )
{	
	RecalculateMatrix();
	UpdateViewProj();
	UpdateFrustumPlanes();
    
   // fprintf(stdout, "Position: %f , %f, %f \n", m_v3Position.x, m_v3Position.y, m_v3Position.z );
   // fprintf(stdout, "View: %f , %f, %f \n",m_v3View.x,m_v3View.y, m_v3View.z );
    
}

void Camera::RecalculateMatrix()
{
	m_matView = glm::lookAt( m_v3Position,  m_v3Position + m_v3View, m_v3Up ); 
	m_v3Side = glm::normalize( glm::cross( m_v3View, m_v3Up ) );
}

AABoundingBox Camera::getWSAABB()
{
	std::vector<glm::vec3> v3Corners = getWSfrustumCorners();
	return AABoundingBox::FromPoints( &v3Corners[ 0 ], 8 );
}

std::vector<glm::vec3> Camera::getWSfrustumCorners()
{
	glm::mat4 matViewInv = GetViewInv();

	//calculate frustum corner coordinates
	float fFov2 = getFovRad() / 2.0f;
	float tanFov2 = glm::tan( fFov2 );
	float h2Far = tanFov2 * m_fFar;
	float h2Near = tanFov2 * m_fNear;
	float hFar = 2.0f * h2Far;
	float hNear = 2.0f * h2Near;
	float aspect = (float) GLRenderer::GetInstance().getScreenWidth() / (float) GLRenderer::GetInstance().getScreenHeight();
	float w2Far = ( hFar * aspect ) / 2.0f;
	float w2Near = ( hNear * aspect ) / 2.0f;

	glm::vec3 v3Corners[ 8 ];

	v3Corners[ 0 ] = glm::vec3( -1.0f * w2Near, -1.0f * h2Near, -m_fNear ); //l,b,n
	v3Corners[ 1 ] = glm::vec3(  1.0f * w2Near, -1.0f * h2Near, -m_fNear ); //r,b,n
	v3Corners[ 2 ] = glm::vec3(  1.0f * w2Near,  1.0f * h2Near, -m_fNear ); //r,t,n
	v3Corners[ 3 ] = glm::vec3( -1.0f * w2Near,  1.0f * h2Near, -m_fNear ); //l,t,n

	v3Corners[ 4 ] = glm::vec3( -1.0f * w2Far, -1.0f * h2Far, -m_fFar ); //l,b,n
	v3Corners[ 5 ] = glm::vec3(  1.0f * w2Far, -1.0f * h2Far, -m_fFar ); //r,b,n
	v3Corners[ 6 ] = glm::vec3(  1.0f * w2Far,  1.0f * h2Far, -m_fFar ); //r,t,n
	v3Corners[ 7 ] = glm::vec3( -1.0f * w2Far,  1.0f * h2Far, -m_fFar ); //l,t,n

	std::vector<glm::vec3> vReturnCorners;

	//transform each corner into WS
	for( int i = 0; i < 8; ++i )
	{
		glm::vec4 v4Result = matViewInv * glm::vec4( v3Corners[ i ], 1.0f );
		vReturnCorners.push_back( glm::vec3( v4Result.x, v4Result.y, v4Result.z ) ); 
	}

	return vReturnCorners;
}

//Frustum-plane calculation implementation as described in "Math for 3D Game Programming & Computer Graphics (Charles River Media Game Development)" by Eric Lengyel
void Camera::UpdateFrustumPlanes()
{
	//LEFT 4th Row + 1st Row
	m_FrustumPlanes[ 0 ].a = m_matViewProj[ 0 ][ 3 ] + m_matViewProj[ 0 ][ 0 ];
	m_FrustumPlanes[ 0 ].b = m_matViewProj[ 1 ][ 3 ] + m_matViewProj[ 1 ][ 0 ];
	m_FrustumPlanes[ 0 ].c = m_matViewProj[ 2 ][ 3 ] + m_matViewProj[ 2 ][ 0 ];
	m_FrustumPlanes[ 0 ].d = m_matViewProj[ 3 ][ 3 ] + m_matViewProj[ 3 ][ 0 ];

	//Right 4th Row - 1st Row
	m_FrustumPlanes[ 1 ].a = m_matViewProj[ 0 ][ 3 ] - m_matViewProj[ 0 ][ 0 ];
	m_FrustumPlanes[ 1 ].b = m_matViewProj[ 1 ][ 3 ] - m_matViewProj[ 1 ][ 0 ];
	m_FrustumPlanes[ 1 ].c = m_matViewProj[ 2 ][ 3 ] - m_matViewProj[ 2 ][ 0 ];
	m_FrustumPlanes[ 1 ].d = m_matViewProj[ 3 ][ 3 ] - m_matViewProj[ 3 ][ 0 ];

	//Bottom 4th Row + 2nd Row
	m_FrustumPlanes[ 2 ].a = m_matViewProj[ 0 ][ 3 ] + m_matViewProj[ 0 ][ 1 ];
	m_FrustumPlanes[ 2 ].b = m_matViewProj[ 1 ][ 3 ] + m_matViewProj[ 1 ][ 1 ];
	m_FrustumPlanes[ 2 ].c = m_matViewProj[ 2 ][ 3 ] + m_matViewProj[ 2 ][ 1 ];
	m_FrustumPlanes[ 2 ].d = m_matViewProj[ 3 ][ 3 ] + m_matViewProj[ 3 ][ 1 ];

	//Top 4th Row - 2nd Row
	m_FrustumPlanes[ 3 ].a = m_matViewProj[ 0 ][ 3 ] - m_matViewProj[ 0 ][ 1 ];
	m_FrustumPlanes[ 3 ].b = m_matViewProj[ 1 ][ 3 ] - m_matViewProj[ 1 ][ 1 ];
	m_FrustumPlanes[ 3 ].c = m_matViewProj[ 2 ][ 3 ] - m_matViewProj[ 2 ][ 1 ];
	m_FrustumPlanes[ 3 ].d = m_matViewProj[ 3 ][ 3 ] - m_matViewProj[ 3 ][ 1 ];

	//Near 4th Row + 3rd Row
	m_FrustumPlanes[ 4 ].a = m_matViewProj[ 0 ][ 3 ] + m_matViewProj[ 0 ][ 2 ];
	m_FrustumPlanes[ 4 ].b = m_matViewProj[ 1 ][ 3 ] + m_matViewProj[ 1 ][ 2 ];
	m_FrustumPlanes[ 4 ].c = m_matViewProj[ 2 ][ 3 ] + m_matViewProj[ 2 ][ 2 ];
	m_FrustumPlanes[ 4 ].d = m_matViewProj[ 3 ][ 3 ] + m_matViewProj[ 3 ][ 2 ];

	//Far 4th Row + 3rd Row
	m_FrustumPlanes[ 5 ].a = m_matViewProj[ 0 ][ 3 ] - m_matViewProj[ 0 ][ 2 ];
	m_FrustumPlanes[ 5 ].b = m_matViewProj[ 1 ][ 3 ] - m_matViewProj[ 1 ][ 2 ];
	m_FrustumPlanes[ 5 ].c = m_matViewProj[ 2 ][ 3 ] - m_matViewProj[ 2 ][ 2 ];
	m_FrustumPlanes[ 5 ].d = m_matViewProj[ 3 ][ 3 ] - m_matViewProj[ 3 ][ 2 ];
	

	//normalize the planes
	for( uint uIdx = 0; uIdx < 6; ++uIdx )
	{
		SFrustumPlane* p = &m_FrustumPlanes[ uIdx ];
		float t = glm::sqrt( p->a * p->a + p->b * p->b + p->c * p->c );
		p->a = p->a / t;
		p->b = p->b / t;
		p->c = p->c / t;
		p->d = p->d / t;
	}
}

bool Camera::IsVisible( const BoundingSphere& rSphereWS )
{
	const glm::vec3& vCenter = rSphereWS.getCenterPoint();
	float fRadius = rSphereWS.getRadius();

	for( uint uIdx = 0; uIdx < 6; ++uIdx )
	{
		if( m_FrustumPlanes[ uIdx ].a * vCenter.x + 
			m_FrustumPlanes[ uIdx ].b * vCenter.y + 
			m_FrustumPlanes[ uIdx ].c * vCenter.z + 
			m_FrustumPlanes[ uIdx ].d <= -fRadius )
		{
			return false;
		}
	}

	return true;
}


void Camera::CheckOriantationMatrix( const uint elapsedTicksMS )
{
	/*glm::mat4 tempM; 
	DomEngine::Input::SMouseInformationSet* currentMState = DomEngine::Input::MouseState::GetCurrentMouseState();

	float angleX = currentMState->m_relativeMouseMovement.x * ->GetMovementMul();
	float angleY = currentMState->m_relativeMouseMovement.y * ->GetMovementMul();

	angleX += currentMState->m_relativeMouseMovement.x * ->GetMovementMul();
	angleY += currentMState->m_relativeMouseMovement.y * ->GetMovementMul();

	m_matView *= glm::gtc::matrix_transform::rotate<float>( tempM, angleX, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	m_matView *= glm::gtc::matrix_transform::rotate<float>( tempM, angleY, glm::vec3( 1.0f, 0.0f, 0.0f ) );*/
}

void Camera::RotateFromMouseMove( float dx, float dy )
{
	RotateViewQuat( dy, m_v3Side );
	RotateViewQuat( dx, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	Update(0);
}

void Camera::RotateViewQuat( const float angle, const glm::vec3 v3Axis )
{
	glm::vec3 v3View = glm::normalize( m_v3View );
	glm::vec3 v3Up = glm::vec3( 0.0f, 1.0f, 0.0f );
	glm::vec3 v3NewView;

	glm::quat quatUp( 0, v3Up );
	glm::quat quatView( 0, v3View );

	glm::quat quatViewResult = glm::rotate( quatView, angle, v3Axis );
	glm::quat quatUpResult = glm::rotate( quatUp, angle, v3Axis );

	v3NewView.x = quatViewResult.x;
	v3NewView.y = quatViewResult.y;
	v3NewView.z = quatViewResult.z;
	v3NewView	= glm::normalize( v3NewView );
	m_v3View	= v3NewView;

	m_v3Up.x	= quatUpResult.x;
	m_v3Up.y	= quatUpResult.y;
	m_v3Up.z	= quatUpResult.z;
	m_v3Up		= glm::normalize( m_v3Up ); 
}

void Camera::MoveForward( float fSpeed )
{
	m_v3Position += m_v3View * fSpeed;
}

void Camera::MoveSideways( float fSpeed )
{
	m_v3Position += m_v3Side * fSpeed;
}