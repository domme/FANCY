#ifndef INCLUDE_CAMERACOMPONENT_H
#define INCLUDE_CAMERACOMPONENT_H

#include "SceneNodeComponent.h"
#include "FixedArray.h"

namespace Fancy { namespace Scene {
  //---------------------------------------------------------------------------//
  class DLLEXPORT CameraComponent : 
    public SceneNodeComponent, public BaseCreator<CameraComponent, SceneNode*>
  {
  public:
    CameraComponent(SceneNode* pOwner);
    virtual ~CameraComponent();

    virtual ObjectName getTypeName() override { return _N(CameraComponent); }
    virtual void serialize(IO::Serializer* aSerializer) override;

    virtual void update() override;
    virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) override;
    virtual ObjectName getTypeName() const override { return _N(Camera); }

    const glm::mat4& getView() const { return m_matView; }
    const glm::mat4& getViewInv() const {return m_matViewInv; }
    const glm::mat4& getProjection() const { return m_matProjection; }
    const glm::mat4& getViewProjection() const { return m_matViewProj; }
    void setFarPlane( float fFar ) { m_fFar = fFar; }
    float getFarPlane() const { return m_fFar; }
    float	getNearPlane() const { return m_fNear; }
    float	getFovRad() const	{ return glm::radians( m_fFovDeg ); }
    float	getFovDeg() const { return m_fFovDeg; }
    float	getAspectRatio() const { return m_fWidth / m_fHeight; }
    void setProjectionPersp( float yFov_deg, float fWidth, float fHeight, float fNear, float fFar );
    void setProjectionOrtho( float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar ); 
    FixedArray<glm::vec3, 8u> getWSfrustumCorners();

  private:
    void updateCameraInternal();

    enum class EFrustumPlane
    {
      PLANE_LEFT = 0,
      PLANE_RIGHT,
      PLANE_BOTTOM,
      PLANE_TOP,
      PLANE_NEAR,
      PLANE_FAR
    };

    glm::mat4 m_matViewInv;
    glm::mat4	m_matView;
    glm::mat4	m_matProjection;
    glm::mat4	m_matViewProj;
    glm::vec4	m_v4FrustumPlanesVS[ 6 ];

    float		m_fFovDeg;
    float		m_fFar;
    float		m_fNear;
    float		m_fFocalLength;
    bool		m_bIsOrtho;
    float		m_fWidth;
    float		m_fHeight;

    void recalculateFrustumPlanes();
    void onViewInvChanged();
    void onProjectionChanged();
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(CameraComponent)
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_CAMERACOMPONENT_H