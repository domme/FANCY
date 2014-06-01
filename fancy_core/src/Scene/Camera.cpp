#include  <math.h>
#include <stdio.h>

#include "../includes.h"
#include "Camera.h"
#include "../Rendering/GLRenderer.h"


Camera::Camera():
m_fMovementSpeed( 50.0f ),
m_fFovDeg( 0.0f ),
m_fFar( 0.0f ),
m_fNear( 0.0f ),
m_matView( 1.0f ),
m_matProjection( 1.0f ),
m_matViewInverse( 1.0f ),
m_matViewProj( 1.0f ),
m_fFocalLength( 0.0f ),
m_bIsOrtho( false ),
m_fWidth( 1.0f ),
m_fHeight( 1.0 )
{

}

Camera::~Camera()
{

}

/*std::stringstream ss;
	ss << m_matView[ 0 ].x << "\t" << m_matView[ 1 ].x << "\t" << m_matView[ 2 ].x << "\t" << m_matView[ 3 ].x << "\n"
	   << m_matView[ 0 ].y << "\t" << m_matView[ 1 ].y << "\t" << m_matView[ 2 ].y << "\t" << m_matView[ 3 ].y << "\n"
	   << m_matView[ 0 ].z << "\t" << m_matView[ 1 ].z << "\t" << m_matView[ 2 ].z << "\t" << m_matView[ 3 ].z << "\n"
	   << m_matView[ 0 ].w << "\t" << m_matView[ 1 ].w << "\t" << m_matView[ 2 ].w << "\t" << m_matView[ 3 ].w << "\n";

	LOG( "Constructed View-Mat:" );
	LOG( ss.str() );
	LOG( "" );
	*/

void Camera::SetOrientation( const glm::vec3& v3Side, const glm::vec3& v3Up, const glm::vec3& v3Forward )
{
	m_matViewInverse[ 0 ] = glm::vec4( v3Side, 0.0f );
	m_matViewInverse[ 1 ] = glm::vec4( v3Up, 0.0f );
	m_matViewInverse[ 2 ] = glm::vec4( v3Forward, 0.0f );

	//TODO: Maybe improve this method by setting the transposed upper 3x3-matrix of the ViewMatrix and then just call paramsChanged()
	//m_matView = glm::inverse( m_matViewInverse );

	//The rotational component of the view-matrix is the transpose of the rotational component of the inverse view-matrix
	m_matView[ 0 ] = glm::vec4( v3Side.x, v3Up.x, v3Forward.x, 0.0f );
	m_matView[ 1 ] = glm::vec4( v3Side.y, v3Up.y, v3Forward.y, 0.0f );
	m_matView[ 2 ] = glm::vec4( v3Side.z, v3Up.z, v3Forward.z, 0.0f ); 

	//Call SetPosition to trigger re-calculation of the translation-component in the view-matrix with respect to the new basis-vectors
	SetPosition( GetPosition() );

}


glm::vec3 Camera::GetSide() const
{
	//Note: assume the camera's view matrix is not scaled (otherwise the return-vec would have to be normalized)
	return glm::vec3( m_matViewInverse[ 0 ] );
}

glm::vec3 Camera::GetUp() const
{
	//Note: assume the camera's view matrix is not scaled (otherwise the return-vec would have to be normalized)
	return glm::vec3( m_matViewInverse[ 1 ] );
}

glm::vec3 Camera::GetForward() const
{
	//Note: assume the camera's view matrix is not scaled (otherwise the return-vec would have to be normalized)
	return glm::vec3( m_matViewInverse[ 2 ] );
}

glm::vec3 Camera::GetPosition() const
{
	return glm::vec3( m_matViewInverse[ 3 ] );
}

void Camera::SetPosition( const glm::vec3& v3Pos )
{
	//Directly set the position to the inverse view-matirx
	m_matViewInverse[ 3 ] = glm::vec4( v3Pos, 1.0f ); 
	
	//Now calculate the inverse translation for the view-matrix as a linear combination of the rotational basis-vectors
	glm::vec3 v3Side = GetSide();
	glm::vec3 v3Up = GetUp();
	glm::vec3 v3Forward = GetForward();

	glm::vec3 v3ViewSide = glm::vec3( v3Side.x, v3Up.x, v3Forward.x );
	glm::vec3 v3ViewUp = glm::vec3( v3Side.y, v3Up.y, v3Forward.y );
	glm::vec3 v3ViewForward = glm::vec3( v3Side.z, v3Up.z, v3Forward.z ); 

	m_matView[ 3 ] = glm::vec4( v3ViewSide * -v3Pos.x + v3ViewUp * -v3Pos.y + v3ViewForward * -v3Pos.z, 1.0f ); 

	paramsChanged();

	std::stringstream ss;
	ss << v3Pos.x << " " << v3Pos.y << " " << v3Pos.z;
	LOG( ss.str() );
}


void Camera::SetView( const glm::mat4& rNewMatView )
{
	m_matView = rNewMatView;
	m_matViewInverse = glm::inverse( m_matView );
	paramsChanged();
}

void Camera::SetProjectionOrtho( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar )
{
	m_matProjection = glm::ortho( fLeft, fRight, fBottom, fTop, fNear, fFar );
	m_fFar = fFar;
	m_fNear = fNear;
	m_fFovDeg = -1.0f; //Not valid in this case -> Mark as negative.
	m_bIsOrtho = true;

	paramsChanged();
}

void Camera::SetProjectionPersp( float yFov_deg, float fWidth, float fHeight, float fNear, float fFar )
{
	m_matProjection = glm::perspectiveFov( yFov_deg, fWidth, fHeight, fNear, fFar );
	m_fNear = fNear;
	m_fFar = fFar;
	m_fFovDeg = yFov_deg;
	m_bIsOrtho = false;
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	//Calculate focal length
	float fFovHor2 = glm::atan( GetAspectRatio() * glm::tan( GetFovRad() / 2.0f ) );
	m_fFocalLength = 1.0f / glm::tan( fFovHor2 );
	
	paramsChanged();
}

void Camera::paramsChanged()
{	
	m_matViewProj = m_matProjection * m_matView;
	updateFrustumPlanes();
}


void Camera::RotateFromMouseMove( float dx, float dy )
{
	//Ugly hack incoming here...
	static bool bFistTime = true;

	if( bFistTime )
	{
		bFistTime = false;
		return;
	}
	//////////////////////////////////////////////////////////////////////////

	rotateViewQuat( dx, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	rotateViewQuat( dy, GetSide() );
}

void Camera::rotateViewQuat( const float angle, const glm::vec3 v3Axis )
{
	glm::vec3 v3View = GetForward();
	glm::vec3 v3Up ( 0.0f, 1.0f, 0.0f );

	glm::quat quatView( 0, v3View );

	glm::quat quatViewResult = glm::rotate( quatView, angle, v3Axis );

	v3View.x = quatViewResult.x;
	v3View.y = quatViewResult.y;
	v3View.z = quatViewResult.z;
	v3View	= glm::normalize( v3View );
	
	glm::vec3 v3Side = glm::cross( v3Up, v3View );
	v3Side = glm::normalize( v3Side );

	v3Up = glm::cross( v3View, v3Side );
	v3Up = glm::normalize( v3Up );

	SetOrientation( v3Side, v3Up, v3View );
}

void Camera::MoveForward( float fSpeed )
{
	SetPosition( GetPosition() - GetForward() * fSpeed );
}

void Camera::MoveSideways( float fSpeed )
{
	SetPosition( GetPosition() + GetSide() * fSpeed );
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
	float fFov2 = GetFovRad() / 2.0f;
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


void Camera::updateFrustumPlanes()
{
	//If the camera's projection matrix is an orthogonal projection, the frustum planes have to be derived 
	//from the general projection matrix
	if( m_bIsOrtho )
	{
		glm::mat4 projT = glm::transpose( m_matProjection );

		m_v4FrustumPlanesVS[ PLANE_NEAR ]		= projT[ 3 ] + projT[ 2 ];
		m_v4FrustumPlanesVS[ PLANE_FAR ]		= projT[ 3 ] - projT[ 2 ];
		m_v4FrustumPlanesVS[ PLANE_LEFT ]		= projT[ 3 ] + projT[ 0 ];
		m_v4FrustumPlanesVS[ PLANE_RIGHT ]		= projT[ 3 ] - projT[ 0 ];
		m_v4FrustumPlanesVS[ PLANE_BOTTOM ]		= projT[ 3 ] + projT[ 1 ]; 
		m_v4FrustumPlanesVS[ PLANE_TOP ]		= projT[ 3 ] - projT[ 1 ];

		//The normals in the plane-vectors (N.x, N.y, N.z, D) have to be normalized
		glm::vec3 v3N;
		for( uint i = 0; i < 6; ++i )
		{
			v3N = glm::normalize( glm::vec3( m_v4FrustumPlanesVS[ i ] ) );
			m_v4FrustumPlanesVS[ i ].x = v3N.x;
			m_v4FrustumPlanesVS[ i ].y = v3N.y;
			m_v4FrustumPlanesVS[ i ].z = v3N.z;
		}	
	}
	
	//If the camera's projection matrix is a perpsective projection, the view-space frustum planes can be
	//determined by the proj-parameters of the camera (more efficient this way)
	else
	{
		float fe1 = glm::sqrt( m_fFocalLength * m_fFocalLength + 1 );
		float fea = glm::sqrt( m_fFocalLength * m_fFocalLength + GetAspectRatio() * GetAspectRatio() );

		m_v4FrustumPlanesVS[ PLANE_NEAR ]		= glm::vec4( 0.0f,						0.0f,									 -1.0f,				 -m_fNear );
		m_v4FrustumPlanesVS[ PLANE_FAR ]		= glm::vec4( 0.0f,						0.0f,									  1.0f,				  m_fFar  );
		m_v4FrustumPlanesVS[ PLANE_LEFT ]		= glm::vec4( m_fFocalLength / fe1,		0.0f,   							-1.0f / fe1,			  0.0f    );
		m_v4FrustumPlanesVS[ PLANE_RIGHT ]		= glm::vec4( -m_fFocalLength / fe1,		0.0f,								-1.0f / fe1,			  0.0f	  );
		m_v4FrustumPlanesVS[ PLANE_BOTTOM ]		= glm::vec4( 0.0f,						m_fFocalLength / fea,	-GetAspectRatio() / fea,			  0.0f	  );
		m_v4FrustumPlanesVS[ PLANE_TOP ]		= glm::vec4( 0.0f,						-m_fFocalLength / fea,	-GetAspectRatio() / fea,			  0.0f	  );
	}
}

bool Camera::IsVisible( const BoundingSphere& rSphereWS ) const
{
	BoundingSphere bSphereVS = rSphereWS * m_matView;
	const glm::vec3& v3Center = bSphereVS.getCenterPoint();
	float fRadius = bSphereVS.getRadius();

	//1) Cull if behind camera
	if( v3Center.z > fRadius )
		return false;

	//2) Cull if behind far plane
	if( glm::abs( v3Center.z ) - m_fFar > fRadius )
		return false;

	//3) Cull if in front of near plane
	if( v3Center.z < 0.0f && m_fNear - glm::abs( v3Center.z ) > fRadius )
		return false;

	//4) Cull if outside for all frustum-planes. NOTE/TODO: PLANE_NEAR and PLANE_FAR should be not needed, because they are already accounted for in step 2) and 3)
	if( glm::dot( v3Center, glm::vec3( m_v4FrustumPlanesVS[ PLANE_LEFT ] ) ) < -fRadius || 
		glm::dot( v3Center, glm::vec3( m_v4FrustumPlanesVS[ PLANE_RIGHT ] ) ) < -fRadius || 
		glm::dot( v3Center, glm::vec3( m_v4FrustumPlanesVS[ PLANE_BOTTOM ] ) ) < -fRadius || 
		glm::dot( v3Center, glm::vec3( m_v4FrustumPlanesVS[ PLANE_TOP ] ) ) < -fRadius )
		return false;

	return true;
}

