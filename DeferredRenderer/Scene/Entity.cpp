#include <Includes.h>
#include <Services/NameRegistry.h>
#include <IO/ModelLoader.h>

#include "Entity.h"

#include "../Engine.h"
#include "SceneNode.h"
#include "SceneManager.h"

Entity::Entity()
{
	
}

Entity::~Entity()
{
	
}

bool Entity::SetMesh( Mesh* pMesh )
{
	m_pMesh = pMesh;
	
	return true;
}

/*
bool Entity::SetMesh( const String& szMeshFilename )
{
	m_pModel = std::move( ModelLoader::GetInstance().LoadModel( szMeshFilename ) );
	return true;
}
*/

void Entity::render()
{

}

void Entity::prepareRender()
{
	if( !m_pMesh || !m_pNode )
	{
		return;
	}

	Engine::GetInstance().GetScene()->m_vCachedRenderObjects.push_back( this );

	updateSceneBounds();
}

void Entity::onAttatchedToNode()
{
	updateSceneBounds();
}

void Entity::updateSceneBounds()
{
	if( !m_pMesh )
		return;

	AABoundingBox worldAABB = m_pMesh->GetAABB();
	worldAABB = worldAABB * m_pNode->getGlobalTransformMAT();


	Engine::GetInstance().GetScene()->updateSceneBounds( worldAABB );
}

const BoundingSphere& Entity::getBoundingSphere()
{
	return m_pMesh->GetBoundingSphere();
}

void Entity::destroyEntity()
{
	detatch();
}