#include "CameraComponent.h"
#include "SceneNode.h"
#include "Renderer.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  CameraComponent::CameraComponent(SceneNode* pOwner)
    : SceneNodeComponent(pOwner),
    m_fFovDeg( 0.0f ),
    m_fFar( 0.0f ),
    m_fNear( 0.0f ),
    m_matViewInv( 1.0f ),
    m_matView( 1.0f ),
    m_matProjection( 1.0f ),
    m_matViewProj( 1.0f ),
    m_fFocalLength( 0.0f ),
    m_bIsOrtho( false ),
    m_fWidth( 1.0f ),
    m_fHeight( 1.0 )
  {

  }
//---------------------------------------------------------------------------//
  CameraComponent::~CameraComponent()
  {

  }
//---------------------------------------------------------------------------//
  void CameraComponent::serialize(IO::Serializer* aSerializer)
  {
    /*aSerializer.serialize(_VAL(m_matViewInv));
    aSerializer.serialize(_VAL(m_matView));
    aSerializer.serialize(_VAL(m_matProjection));
    aSerializer.serialize(_VAL(m_matViewProj));
    aSerializer.serialize(_VAL(m_fFovDeg));
    aSerializer.serialize(_VAL(m_fFar));
    aSerializer.serialize(_VAL(m_fNear));
    aSerializer.serialize(_VAL(m_fFocalLength));
    aSerializer.serialize(_VAL(m_bIsOrtho));
    aSerializer.serialize(_VAL(m_fWidth));
    aSerializer.serialize(_VAL(m_fHeight));*/
  }
//---------------------------------------------------------------------------//
  void CameraComponent::update()
  {
    updateCameraInternal();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::gatherRenderItems( SceneRenderDescription* pRenderDesc )
  {

  }
//---------------------------------------------------------------------------//
  void CameraComponent::updateCameraInternal()
  {
    const glm::mat4& viewInverse = m_pOwner->getTransform().getCachedWorld();
    m_matViewInv = viewInverse;
    onViewInvChanged();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::setProjectionOrtho( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar )
  {
    m_matProjection = glm::ortho( fLeft, fRight, fBottom, fTop, fNear, fFar );
    m_fFar = fFar;
    m_fNear = fNear;
    m_fFovDeg = -1.0f; //Not valid in this case -> Mark as negative.
    m_bIsOrtho = true;

    onProjectionChanged();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::setProjectionPersp( float yFov_deg, float fWidth, float fHeight, float fNear, float fFar )
  {
    m_matProjection = glm::perspectiveFov( yFov_deg, fWidth, fHeight, fNear, fFar );
    m_fNear = fNear;
    m_fFar = fFar;
    m_fFovDeg = yFov_deg;
    m_bIsOrtho = false;
    m_fWidth = fWidth;
    m_fHeight = fHeight;

    //Calculate focal length
    float fFovHor2 = glm::atan( getAspectRatio() * glm::tan( getFovRad() / 2.0f ) );
    m_fFocalLength = 1.0f / glm::tan( fFovHor2 );

    onProjectionChanged();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::onProjectionChanged()
  {	
    m_matViewProj = m_matProjection * m_matView;
    recalculateFrustumPlanes();
  }
//---------------------------------------------------------------------------//
  void CameraComponent::onViewInvChanged()
  {
    m_matView = glm::inverse(m_matViewInv);
    m_matViewProj = m_matProjection * m_matView;
    recalculateFrustumPlanes();
  }
//---------------------------------------------------------------------------//
  FixedArray<glm::vec3, 8u> CameraComponent::getWSfrustumCorners()
  {
    //calculate frustum corner coordinates
    float fFov2 = getFovRad() * 0.5f;
    float tanFov2 = glm::tan( fFov2 );
    float h2Far = tanFov2 * m_fFar;
    float h2Near = tanFov2 * m_fNear;
    float hFar = 2.0f * h2Far;
    float hNear = 2.0f * h2Near;
    
    float aspect = (float) Rendering::Renderer::getInstance().getViewport().z /
      (float) Rendering::Renderer::getInstance().getViewport().w;
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

    FixedArray<glm::vec3, 8u> vReturnCorners;
    vReturnCorners.resize(8);

    //transform each corner into WS
    for( int i = 0; i < 8; ++i )
    {
      vReturnCorners[i] = glm::vec3( m_matViewInv * glm::vec4( v3Corners[ i ], 1.0f ) ); 
    }

    return vReturnCorners;
  }
//---------------------------------------------------------------------------//
  void CameraComponent::recalculateFrustumPlanes()
  {
    //If the camera's projection matrix is an orthogonal projection, the frustum planes have to be derived 
    //from the general projection matrix
    if( m_bIsOrtho )
    {
      glm::mat4 projT = glm::transpose( m_matProjection );

      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_NEAR]	= projT[ 3 ] + projT[ 2 ];
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_FAR] = projT[ 3 ] - projT[ 2 ];
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_LEFT]	= projT[ 3 ] + projT[ 0 ];
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_RIGHT] = projT[ 3 ] - projT[ 0 ];
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_BOTTOM]	= projT[ 3 ] + projT[ 1 ]; 
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_TOP]	= projT[ 3 ] - projT[ 1 ];

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
      float fea = glm::sqrt( m_fFocalLength * m_fFocalLength + getAspectRatio() * getAspectRatio() );

      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_NEAR ]		= glm::vec4( 0.0f,						        0.0f,									-1.0f,				 -m_fNear );
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_FAR ]		  = glm::vec4( 0.0f,						        0.0f,									 1.0f,				  m_fFar  );
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_LEFT ]		= glm::vec4( m_fFocalLength / fe1,		0.0f,   							-1.0f / fe1,			  0.0f    );
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_RIGHT ]		= glm::vec4( -m_fFocalLength / fe1,		0.0f,								  -1.0f / fe1,			  0.0f	  );
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_BOTTOM ]	= glm::vec4( 0.0f,						m_fFocalLength / fea,	-getAspectRatio() / fea,		0.0f	  );
      m_v4FrustumPlanesVS[(uint) EFrustumPlane::PLANE_TOP ]		  = glm::vec4( 0.0f,						-m_fFocalLength / fea,	-getAspectRatio() / fea,	0.0f	  );
    }
  }
//---------------------------------------------------------------------------//
  /*bool CameraComponent::IsVisible( const BoundingSphere& rSphereWS ) const
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
  } */
} }  // end of namespace Fancy::Scene