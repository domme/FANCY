#include "PointLight.h"
#include "../Scene/SceneNode.h"

PointLight::PointLight() : Light(),
m_fFalloffStart( 20.0f ),
m_fFalloffEnd( 100.0f )
{
	m_eLightType = Light::LIGHTTYPE_POINT;
}

PointLight::~PointLight()
{

}

void PointLight::update()
{
	//Warning: Hacked for volume rendering assignment! TODO:Implement properly!

	glm::vec4 v4Pos = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	v4Pos = m_pNode->getGlobalTransformMAT() * v4Pos;

	glm::vec3 v3Pos = glm::vec3( v4Pos.x, v4Pos.y, v4Pos.z );

	m_clLightViewCamera.SetView( glm::lookAt( v3Pos, glm::vec3( 0.0f, 0.0f, -3.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) );
	m_clLightViewCamera.InitPerspectiveProjection( 60.0f, 1.0f, 1.0f, 10.0f );

}
