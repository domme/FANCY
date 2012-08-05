#include "DirectionalLight.h"
#include "../Engine.h"
#include "../Scene/Camera.h"
#include "../Scene/AABoundingBox.h"
#include "../Scene/SceneNode.h"

DirectionalLight::DirectionalLight() : Light()
{
	m_eLightType = Light::LIGHTTYPE_DIRECTIONAL;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::update()
{
	glm::mat4 transform = m_pNode->getGlobalTransformMAT();
	AABoundingBox clSceneAABB_WS = m_pEngine->GetScene()->getSceneBoundsWS();
	//cancel out original translation
	transform[3][0] = 0.0f;
	transform[3][1] = 0.0f;
	transform[3][2] = 0.0f;

	//offset camera pos by a fixed amount in the negative light directionli
	float fOffset = 1000;
	glm::mat4 translation = glm::translate( -m_v3CachedDirection * fOffset );
	transform = translation * transform;

	glm::mat4 m4LightView = glm::inverse( transform );
	m_clLightViewCamera.SetView( m4LightView );

	//bring Scene-bounding box into light-space
	AABoundingBox clSceneAABB_LS = clSceneAABB_WS * m4LightView;
	
	//create projection matrix
	//m_clLightViewCamera.InitOrthogonalProjection( -5.0f, 5.0f, -5.0f, 5.0f, 1.0f, 1000.0f );
	m_clLightViewCamera.InitOrthogonalProjection( clSceneAABB_LS.m_v3MinPos.x, clSceneAABB_LS.m_v3MaxPos.x, clSceneAABB_LS.m_v3MinPos.y, clSceneAABB_LS.m_v3MaxPos.y, 1.0f,  1000.0f );

	
}


void DirectionalLight::renderShadowMap()
{	
	
}