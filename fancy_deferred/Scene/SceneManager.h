#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <Services/NameRegistry.h>
#include <IO/PathService.h>
#include <Scene/AABoundingBox.h>

#include "SceneNode.h"
#include "Entity.h"

#include "NodeRegistry.h"
#include "ObjectRegistry.h"

class Light;
class DirectionalLight;
class SpotLight;
class PointLight;
class VolumeEntity;

struct DLLEXPORT SLightCacheEntry
{
	SLightCacheEntry( Light* _pLight, SceneNode* _pNode )
	{
		pLight = _pLight;
		pNode = _pNode;
	}

	Light* pLight;
	SceneNode* pNode;
};

class DLLEXPORT  SceneManager
{
	friend class Entity;
	friend class VolumeEntity;

	public:		
		SceneManager();
		virtual ~SceneManager();


		void									OnResolutionChanged( glm::ivec2 res ); 

		Entity*									CreateEntity( Mesh* pMesh );
		//Entity*									CreateEntity( const String& szName, const String& szModelFileNameRelative );
		PointLight*								createPointLight( const String& szName, const glm::vec3& v3LightColor = glm::vec3( 1.0f, 1.0f, 1.0f ) , float fIntenisty = 1.0f, float fFalloffDistanceStart = 10.0f, float fFalloffDistanceEnd = 80.0f );
		DirectionalLight*						createDirectionalLight( const String& szName, const glm::vec3& v3LightColor = glm::vec3( 1.0f, 1.0f, 1.0f ), float fIntensity = 1.0f );
		SpotLight*								createSpotLight( const String& szName, const glm::vec3& v3LightColor = glm::vec3( 1.0f, 1.0f, 1.0f ), float fIntensity = 1.0f, float fUmbraAngleDeg = 20.0f, float fPenumbraAngleDeg = 5.0f, float fExponent = 1.0f );

		SceneNode*								LoadAssetIntoScene( const String& szPath );


		std::vector<Entity*>&					GetRenderObjects()					{ return m_vCachedRenderObjects; }
		std::vector<VolumeEntity*>&				GetVolumeObjects()					{ return m_vCachedVolumeObjects; }
		const std::vector<Light*>&				getCachedLights()			 const	{ return m_vCachedLights; }
		const std::vector<PointLight*>&			getCachedPointLights()		 const	{ return m_vCachedPointLights; }
		const std::vector<DirectionalLight*>&	getCachedDirectionalLights() const	{ return m_vCachedDirectionalLights; }
		const std::vector<SpotLight*>&			getCachedSpotLights()		 const	{ return m_vChachedSpotLights; }
		
		bool									attatchToScene( BaseRenderableObject* pObject, const String& szNodeName = "root" );	
		bool									detatchFromScene( BaseRenderableObject* pObject );
		bool									detatchFromScene( const String& szObjectName );
		SceneNode*								getRootNode();
		const SceneNode* const					getRootNode() const					{ return m_pRootNode; }
		void									prepareRender();
		void									updateSceneBounds( const AABoundingBox& rAABB );
		const AABoundingBox&					getSceneBoundsWS() const			{ return m_clSceneBounds; }
		float									getMaxSceneSize() const				{ return m_fMaxSceneSize; }
		String									GenerateNextUniqueMeshName();
		void									AddLightToRenderCache( Light* pLight, SceneNode* pNode );
		void									SetCamera( Camera* pCamera )		{ if( m_pCamera ) SAFE_DELETE( m_pCamera ); m_pCamera = pCamera; }
		Camera*									GetCamera()							{ return m_pCamera; }
		const Camera*							GetCamera() const					{ return m_pCamera; }


		
	private:
		void									 generalInit();
		void									 preprocessLights();
		void									 preprocessLight( Light* pLight, SceneNode* pNode );
		void									 preprocessPointLight( PointLight* pPointLight, SceneNode* pNode );
		void									 preprocessSpotLight( SpotLight* pSpotLight, SceneNode* pNode );
		void									 preprocessDirectionalLight( DirectionalLight* pDirLight, SceneNode* pNode );

		SceneNode*								m_pRootNode;
		NodeRegistry*							m_pNodeRegistry;
		ObjectRegistry*							m_pObjectRegistry;	
		LightRegistry*							m_pLightRegistry;
		std::vector<SLightCacheEntry>			m_vCachedLightEntries;
		std::vector<Light*>						m_vCachedLights;
		float									m_fMaxSceneSize;
		uint									m_uGenericMeshCounter;
		
		std::vector<PointLight*>				m_vCachedPointLights;
		std::vector<DirectionalLight*>			m_vCachedDirectionalLights;
		std::vector<SpotLight*>					m_vChachedSpotLights;
		std::vector<Entity*>					m_vCachedRenderObjects;
		std::vector<VolumeEntity*>				m_vCachedVolumeObjects;


		AABoundingBox							m_clSceneBounds;

		Camera*									m_pCamera;
};


#endif