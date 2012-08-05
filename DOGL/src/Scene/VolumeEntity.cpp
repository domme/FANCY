#include "../includes.h"

#include "VolumeEntity.h"

#include "../Engine.h"
#include "SceneNode.h"
#include "../IO/OBJimporter.h"
#include "SceneManager.h"
#include "../Services/NameRegistry.h"
#include "../IO/ModelLoader.h"

VolumeEntity::VolumeEntity()
{
	
}

VolumeEntity::~VolumeEntity()
{
	
}

bool VolumeEntity::SetVolumeMesh( std::unique_ptr<VolumeMesh> pMesh )
{
	m_pVolumeMesh = std::move( pMesh );
	
	return true;
}

bool VolumeEntity::SetVolumeMesh( const String& szBaseTexPath, uint uTexStartIndex, uint uTexEndIndex, const String& szExtension, VolumeMaterial* pVolumeMaterial, const String& szTransferFuktionPath )
{
	std::unique_ptr<VolumeMesh> pVolMesh( new VolumeMesh );
	pVolMesh->Create( szBaseTexPath, uTexStartIndex, uTexEndIndex, szExtension, pVolumeMaterial, szTransferFuktionPath );

	m_pVolumeMesh = std::move( pVolMesh );

	return true;
}

/*
bool Entity::SetMesh( const String& szMeshFilename )
{
	m_pModel = std::move( ModelLoader::GetInstance().LoadModel( szMeshFilename ) );
	return true;
}
*/

void VolumeEntity::render()
{

}

void VolumeEntity::prepareRender()
{
	if( !m_pVolumeMesh || !m_pNode )
	{
		return;
	}

	Engine::GetInstance().GetScene()->m_vCachedVolumeObjects.push_back( this );

	updateSceneBounds();
}

void VolumeEntity::onAttatchedToNode()
{
	updateSceneBounds();
}

void VolumeEntity::updateSceneBounds()
{
	if( !m_pVolumeMesh )
		return;

	AABoundingBox worldAABB = m_pVolumeMesh->GetAABB();
	worldAABB = worldAABB * m_pNode->getGlobalTransformMAT();


	Engine::GetInstance().GetScene()->updateSceneBounds( worldAABB );
}


const BoundingSphere& VolumeEntity::getBoundingSphere()
{
	//TODO: Make more efficient! (propably cache it within the same frame?)
	return ( m_pVolumeMesh->GetBoundingSphere() * m_pNode->getGlobalTransformMAT() );
}

void VolumeEntity::destroyEntity()
{
	detatch();
}