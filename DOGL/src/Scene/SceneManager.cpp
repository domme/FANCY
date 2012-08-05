#include "../includes.h"
#include "SceneManager.h"
#include "../Engine.h"
#include "../IO/ModelLoader.h"

#include "../Light/DirectionalLight.h"
#include "../Light/PointLight.h"
#include "../Light/SpotLight.h"

#include "../Rendering/Materials/MAT_FSquad_Textured3D.h"
#include "../Rendering/Materials/MAT_VolCube_Raycast_Simple.h"

SceneManager::SceneManager() :
m_fMaxSceneSize( 0 ),
m_uGenericMeshCounter( 0 )
{
	generalInit();
}

SceneManager::~SceneManager()
{
	m_pObjectRegistry->destroyRegistry();
	m_pNodeRegistry->destroyRegistry();
	m_pLightRegistry->destroyRegistry();
}

void SceneManager::generalInit()
{
	m_pObjectRegistry	= &ObjectRegistry::getInstance();
	m_pNodeRegistry		= &NodeRegistry::getInstance();
	m_pLightRegistry	= &LightRegistry::getInstance();
	m_pObjImporter		= &OBJimporter::getInstance();

	//create root node
	m_pRootNode = new SceneNode( "root" );
	m_pNodeRegistry->registerObject( m_pRootNode->getName(), m_pRootNode );	
}

void SceneManager::prepareRender()
{
	//reset the Scene Bounds
	m_clSceneBounds = AABoundingBox( glm::vec3( -1.0f, -1.0f, -1.0f ), glm::vec3( 1.0f, 1.0f, 1.0f ) );

	m_vCachedRenderObjects.clear();
	m_vCachedVolumeObjects.clear();

	m_pRootNode->prepareRender();
	gatherAndPreprocessLights();
}

void SceneManager::gatherAndPreprocessLights()
{
	m_vCachedLights.clear();
	m_vCachedPointLights.clear();
	m_vChachedSpotLights.clear();
	m_vCachedDirectionalLights.clear();


	m_pLightRegistry->collectAllRegisteredObjects( m_vCachedLights );

	for( uint uIdx = 0; uIdx <  m_vCachedLights.size(); ++uIdx )
	{
		preprocessLight( m_vCachedLights[ uIdx ] );
	}

	//Reorder the cached lights so that they are ordered by their lighttypes
	m_vCachedLights.clear();
	m_vCachedLights.insert( m_vCachedLights.end(), m_vCachedDirectionalLights.begin(), m_vCachedDirectionalLights.end() );
	m_vCachedLights.insert( m_vCachedLights.end(), m_vCachedPointLights.begin(), m_vCachedPointLights.end() );
	m_vCachedLights.insert( m_vCachedLights.end(), m_vChachedSpotLights.begin(), m_vChachedSpotLights.end() );
}

void SceneManager::preprocessLight( Light* pLight )
{
	if( !pLight ) 
	{
		return;
	}

	if( !pLight->isAttatched() )
	{
		return;
	}

	glm::vec4 vLocalPosition( 0.0f, 0.0f, 0.0f, 1.0f );
	const glm::mat4& nodeMat = pLight->getNode()->getGlobalTransformMAT();

	vLocalPosition = nodeMat * vLocalPosition; //vLocalPosition now contains the current Position of this light
	glm::vec3 vCachedPos = glm::vec3( vLocalPosition.x, vLocalPosition.y, vLocalPosition.z );
	pLight->setChachedPosition( vCachedPos );

	Light::ELightTpye eLightType = pLight->getLightType();

	switch( eLightType )
	{
	case Light::LIGHTTYPE_DIRECTIONAL:
		preprocessDirectionalLight( (DirectionalLight*) pLight );
		break;

	case Light::LIGHTTYPE_POINT:
		preprocessPointLight( (PointLight*) pLight );
		break;

	case Light::LIGHTTYPE_SPOT:
		preprocessSpotLight( (SpotLight*) pLight );
		break;

	default:
		break;
	}

	//Handle all update tasks of the light if it has any...
	pLight->update();
}

void SceneManager::preprocessPointLight( PointLight* pPointLight )
{
	m_vCachedPointLights.push_back( pPointLight );

	pPointLight->renderShadowMap();
}

void SceneManager::preprocessSpotLight( SpotLight* pSpotLight )
{
	m_vChachedSpotLights.push_back( pSpotLight );

	glm::vec4 v4LocalZ = glm::vec4( 0.0f, 0.0f, -1.0f, 0.0f );
	glm::vec4 v4Direction = pSpotLight->getNode()->getGlobalTransformMAT() * v4LocalZ; 
	pSpotLight->setCachedDirection(  glm::normalize( glm::vec3( v4Direction.x, v4Direction.y, v4Direction.z ) ) );

	pSpotLight->renderShadowMap();
}

void SceneManager::preprocessDirectionalLight( DirectionalLight* pDirLight )
{
	m_vCachedDirectionalLights.push_back( pDirLight );

	glm::vec4 v4LocalZ = glm::vec4( 0.0f, 0.0f, -1.0f, 0.0f );
	glm::vec4 v4Direction = pDirLight->getNode()->getGlobalTransformMAT() * v4LocalZ; 
	pDirLight->setCachedDirection( glm::normalize( glm::vec3( v4Direction.x, v4Direction.y, v4Direction.z ) ) );

	pDirLight->renderShadowMap();
}

SceneNode* SceneManager::getRootNode() 
{
	return m_pRootNode;
}

SceneNode* SceneManager::LoadAssetIntoScene( const String& szPath )
{
	SceneNode* pNode = ModelLoader::GetInstance().LoadAsset( szPath, this );

	m_pRootNode->AppendChildSceneNode( pNode );

	return pNode;
}

Entity* SceneManager::CreateEntity( std::unique_ptr<Mesh> pMesh )
{
	if( !pMesh )
	{
		return NULL;
	}

	Entity* pNewEntity = new Entity(  );
	pNewEntity->SetMesh( std::move( pMesh ) );

	return pNewEntity;
}

//Entity* SceneManager::CreateEntity( const String& szName, const String& szModelFileNameRelative )
//{
//    if( m_pObjectRegistry->isObjectRegistered( szName ) )
//    {
//	   return NULL;
//    }
//	
//	unique_ptr<Model> pModel = std::move( ModelLoader::GetInstance().LoadModel( szModelFileNameRelative ) );
//
//	return CreateEntity( szName, std::move( pModel ) );
//}

VolumeEntity* SceneManager::CreateVolumeEntity( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension, const String& szTransferFuktionPath )
{
	//MAT_FSquad_Textured3D* pVolMat = new MAT_FSquad_Textured3D();
	MAT_VolCube_Raycast_Simple* pVolMat = new MAT_VolCube_Raycast_Simple();
	pVolMat->Init();
	
	VolumeEntity* pNewVolEnt = new VolumeEntity();
	pNewVolEnt->SetVolumeMesh( szBaseTexPath, uStartIndex, uEndIndex, szExtension, pVolMat, szTransferFuktionPath );

	return pNewVolEnt;
}

PointLight*	SceneManager::createPointLight( const String& szName, const glm::vec3& v3LightColor /* = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f )*/ , float fIntenisty /* = 1.0f */, float fFalloffDistanceStart /* = 10.0f */, float fFalloffDistanceEnd /* = 50.0f*/ )
{
	if( m_pLightRegistry->isObjectRegistered( szName ) )
	{
		return NULL;
	}
		

	PointLight* pNewLight = new PointLight();
	pNewLight->setColor( v3LightColor );
	pNewLight->setIntensity( fIntenisty );
	pNewLight->setFalloffStart( fFalloffDistanceStart );
	pNewLight->setFalloffEnd( fFalloffDistanceEnd );
	m_pLightRegistry->registerObject( szName, pNewLight );

	return pNewLight;
}

DirectionalLight* SceneManager::createDirectionalLight( const String& szName, const glm::vec3& v3LightColor /* = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f )*/, float fIntensity /*= 1.0f*/ )
{
	if( m_pLightRegistry->isObjectRegistered( szName ) )
	{
		return NULL;
	}

	
	DirectionalLight* pNewLight = new DirectionalLight();
	pNewLight->setColor( v3LightColor );
	pNewLight->setIntensity( fIntensity );
	m_pLightRegistry->registerObject( szName, pNewLight );

	return pNewLight;
}

SpotLight* SceneManager::createSpotLight( const String& szName, const glm::vec3& v3LightColor /*= glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f )*/, float fIntensity /*= 1.0f*/, float fUmbraAngleDeg /*= 5.0f*/, float fPenumbraAngleDeg /*= 30.0f*/, float fExponent /*= 2.0f*/ )
{
	if( m_pLightRegistry->isObjectRegistered( szName ) )
	{
		return NULL;
	}
		

	SpotLight* pSpotLight = new SpotLight();
	pSpotLight->setColor( v3LightColor );
	pSpotLight->setIntensity( fIntensity );
	pSpotLight->setPenumbraAngleDeg( fPenumbraAngleDeg );
	pSpotLight->setUmbraAngleDeg( fUmbraAngleDeg );
	pSpotLight->setExponent( fExponent );
	m_pLightRegistry->registerObject( szName, pSpotLight );

	return pSpotLight;
}

void SceneManager::updateSceneBounds( const AABoundingBox& rAABB )
{
	m_clSceneBounds = m_clSceneBounds.combine( rAABB );
    m_fMaxSceneSize = glm::length( m_clSceneBounds.m_v3MaxPos - m_clSceneBounds.m_v3MinPos );
}


bool SceneManager::attatchToScene( BaseRenderableObject* pObject, const String& szNodeName /*= "root" */ )
{
	if( !pObject )
	{
		return false;
	}

	if( pObject->isAttatched() )
	{
		return false;
	}

	SceneNode* pNode = m_pNodeRegistry->getObject( szNodeName );
	
	if( !pNode )
	{
		return false;
	}
	
    return pNode->attatchEntity( pObject );
}


bool SceneManager::detatchFromScene( BaseRenderableObject* pObject )
{
	if( !pObject )
	{
		return false;
	}
	
	SceneNode* pNode = pObject->getNode();
	
	if( !pObject->isAttatched() || !pNode )
	{
		return false;
	}
	
    return pNode->removeEntity( pObject );
}

bool SceneManager::detatchFromScene( const String& szObjectName )
{
	BaseRenderableObject* pObject = m_pObjectRegistry->getObject( szObjectName );
	return detatchFromScene( pObject );
}

String SceneManager::GenerateNextUniqueMeshName() 
{
	std::stringstream ss;
	ss << "Mesh_" << m_uGenericMeshCounter++;

	return ss.str();
}
