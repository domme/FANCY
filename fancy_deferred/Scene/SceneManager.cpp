#include <Includes.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>

#include "SceneManager.h"
#include "SceneLoader.h"
#include "../Engine.h"

SceneManager::SceneManager() :
m_fMaxSceneSize( 0 ),
m_uGenericMeshCounter( 0 ),
m_pCamera( NULL ),
m_pRootNode( NULL ),
m_pLightRegistry( NULL ),
m_pNodeRegistry( NULL ),
m_pObjectRegistry( NULL ),
m_clSceneBounds( glm::vec3( -1.0f, -1.0f, -1.0f ), glm::vec3( 1.0f, 1.0f, 1.0f ) )
{
	generalInit();
}

SceneManager::~SceneManager()
{
	m_pObjectRegistry->destroyRegistry();
	m_pNodeRegistry->destroyRegistry();
	m_pLightRegistry->destroyRegistry();

	SAFE_DELETE( m_pRootNode );
	SAFE_DELETE( m_pCamera ); 
}

void SceneManager::generalInit()
{
	m_pObjectRegistry	= &ObjectRegistry::getInstance();
	m_pNodeRegistry		= &NodeRegistry::getInstance();
	m_pLightRegistry	= &LightRegistry::getInstance();

	//create root node
	m_pRootNode = new SceneNode( "root" );
	m_pRootNode->SetScene( this ); 
	m_pNodeRegistry->registerObject( m_pRootNode->getName(), m_pRootNode );	

	m_pCamera = new Camera();
	glm::vec3 v3Eye( 0.0f, 0.0f, 0.0f );
	glm::vec3 v3At( 0.0f, 0.0f, -1.0f );
	glm::vec3 v3Up( 0.0f, 1.0f, 0.0f );
	m_pCamera->SetView( glm::lookAt( v3Eye, v3At, v3Up ) );
}


void SceneManager::OnResolutionChanged( glm::ivec2 res )
{
	m_pCamera->SetProjectionPersp( 60.0f, (float) res.x, (float) res.y, 1.0f, 50.0f );
}


void SceneManager::prepareRender()
{
	//reset the Scene Bounds
	m_clSceneBounds = AABoundingBox( glm::vec3( -1.0f, -1.0f, -1.0f ), glm::vec3( 1.0f, 1.0f, 1.0f ) );

	m_vCachedRenderObjects.clear();
	m_vCachedVolumeObjects.clear();

	m_vCachedLightEntries.clear();
	m_pRootNode->prepareRender();
	preprocessLights();
}

void SceneManager::preprocessLights()
{
	m_vCachedPointLights.clear();
	m_vChachedSpotLights.clear();
	m_vCachedDirectionalLights.clear();
	

	for( uint uIdx = 0; uIdx < m_vCachedLightEntries.size(); ++uIdx )
	{
		preprocessLight( m_vCachedLightEntries[ uIdx ].pLight, m_vCachedLightEntries[ uIdx ].pNode );
	}

	//Reorder the cached lights so that they are ordered by their lighttypes
	m_vCachedLights.clear();
	m_vCachedLights.insert( m_vCachedLights.end(), m_vCachedDirectionalLights.begin(), m_vCachedDirectionalLights.end() );
	m_vCachedLights.insert( m_vCachedLights.end(), m_vCachedPointLights.begin(), m_vCachedPointLights.end() );
	m_vCachedLights.insert( m_vCachedLights.end(), m_vChachedSpotLights.begin(), m_vChachedSpotLights.end() );
}

void SceneManager::preprocessLight( Light* pLight, SceneNode* pNode )
{
	if( !pLight ) 
	{
		return;
	}
	
	Light::ELightTpye eLightType = pLight->GetLightType();

	switch( eLightType )
	{
	case Light::LIGHTTYPE_DIRECTIONAL:
		preprocessDirectionalLight( (DirectionalLight*) pLight, pNode );
		break;

	case Light::LIGHTTYPE_POINT:
		preprocessPointLight( (PointLight*) pLight, pNode );
		break;

	case Light::LIGHTTYPE_SPOT:
		preprocessSpotLight( (SpotLight*) pLight, pNode );
		break;

	default:
		break;
	}

	//Handle all update tasks of the light if it has any...
	pLight->Update();
}

void SceneManager::preprocessPointLight( PointLight* pPointLight, SceneNode* pNode )
{
	const glm::mat4& rLightMat = pNode->getGlobalTransformMAT();
	pPointLight->SetPosition( glm::vec3( rLightMat[ 3 ][ 0 ], rLightMat[ 3 ][ 1 ], rLightMat[ 3 ][ 2 ] ) );
	
	m_vCachedPointLights.push_back( pPointLight );
}

void SceneManager::preprocessSpotLight( SpotLight* pSpotLight, SceneNode* pNode )
{
	const glm::mat4 rLightMat = pNode->getGlobalTransformMAT();
	pSpotLight->setDirection( glm::normalize( glm::vec3( rLightMat[ 2 ][ 0 ], rLightMat[ 2 ][ 1 ], rLightMat[ 2 ][ 2 ] ) ) );

	m_vChachedSpotLights.push_back( pSpotLight );
}

void SceneManager::preprocessDirectionalLight( DirectionalLight* pDirLight, SceneNode* pNode )
{
	m_vCachedDirectionalLights.push_back( pDirLight );

	const glm::mat4 rLightMat = pNode->getGlobalTransformMAT(); 
	pDirLight->setDirection( glm::normalize( glm::vec3( rLightMat[ 2 ][ 0 ], rLightMat[ 2 ][ 1 ], rLightMat[ 2 ][ 2 ] ) ) );
}

void SceneManager::AddLightToRenderCache( Light* pLight, SceneNode* pNode )
{
	m_vCachedLightEntries.push_back( SLightCacheEntry( pLight, pNode ) );
}

SceneNode* SceneManager::getRootNode() 
{
	return m_pRootNode;
}

SceneNode* SceneManager::LoadAssetIntoScene( const String& szPath )
{
	SceneNode* pNode = SceneLoader::GetInstance().LoadAsset( szPath, this );

	m_pRootNode->AppendChildSceneNode( pNode );

	return pNode;
} 

Entity* SceneManager::CreateEntity( Mesh* pMesh )
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

/*VolumeEntity* SceneManager::CreateVolumeEntity( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension, const String& szTransferFuktionPath )
{
	//MAT_FSquad_Textured3D* pVolMat = new MAT_FSquad_Textured3D();
	MAT_VolCube_Raycast_Simple* pVolMat = new MAT_VolCube_Raycast_Simple();
	pVolMat->Init();
	
	VolumeEntity* pNewVolEnt = new VolumeEntity();
	pNewVolEnt->SetVolumeMesh( szBaseTexPath, uStartIndex, uEndIndex, szExtension, pVolMat, szTransferFuktionPath );

	return pNewVolEnt;
} */

PointLight*	SceneManager::createPointLight( const String& szName, const glm::vec3& v3LightColor /* = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f )*/ , float fIntenisty /* = 1.0f */, float fFalloffDistanceStart /* = 10.0f */, float fFalloffDistanceEnd /* = 50.0f*/ )
{
	if( m_pLightRegistry->isObjectRegistered( szName ) )
	{
		return NULL;
	}
		

	PointLight* pNewLight = new PointLight();
	pNewLight->SetColor( v3LightColor );
	pNewLight->SetIntensity( fIntenisty );
	pNewLight->setFalloffStart( fFalloffDistanceStart );
	pNewLight->setFalloffEnd( fFalloffDistanceEnd );
	pNewLight->Init();

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
	pNewLight->SetColor( v3LightColor );
	pNewLight->SetIntensity( fIntensity );

	pNewLight->Init();
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
	
	pSpotLight->SetColor( v3LightColor );
	pSpotLight->SetIntensity( fIntensity );
	pSpotLight->setPenumbraAngleDeg( fPenumbraAngleDeg );
	pSpotLight->setUmbraAngleDeg( fUmbraAngleDeg );
	pSpotLight->setExponent( fExponent );

	pSpotLight->Init();
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
