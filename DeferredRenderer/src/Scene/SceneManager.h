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

class  SceneManager
{
	friend class Entity;
	friend class VolumeEntity;

	public:		
		SceneManager();
		virtual ~SceneManager();

		Entity*									CreateEntity( std::unique_ptr<Mesh> pMesh );
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
		
	private:
		void									 generalInit();
		void									 gatherAndPreprocessLights();
		void									 preprocessLight( Light* pLight );
		void									 preprocessPointLight( PointLight* pPointLight );
		void									 preprocessSpotLight( SpotLight* pSpotLight );
		void									 preprocessDirectionalLight( DirectionalLight* pDirLight );

		SceneNode*								m_pRootNode;
		NodeRegistry*							m_pNodeRegistry;
		ObjectRegistry*							m_pObjectRegistry;	
		LightRegistry*							m_pLightRegistry;
		std::vector<Light*>						m_vCachedLights;
		float									m_fMaxSceneSize;
		uint									m_uGenericMeshCounter;

		std::vector<PointLight*>				m_vCachedPointLights;
		std::vector<DirectionalLight*>			m_vCachedDirectionalLights;
		std::vector<SpotLight*>					m_vChachedSpotLights;
		std::vector<Entity*>					m_vCachedRenderObjects;
		std::vector<VolumeEntity*>				m_vCachedVolumeObjects;

		AABoundingBox							m_clSceneBounds;
};


#endif