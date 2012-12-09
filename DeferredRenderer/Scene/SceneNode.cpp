#include <includes.h>
#include <Math/QuaternionService.h>

#include "SceneNode.h"
#include "SceneManager.h"
#include "NodeRegistry.h"

#include <algorithm>

SceneNode::SceneNode( const String& szName ) :
m_pParentNode( NULL ),
m_pParentScene( NULL ), 
m_szName( "" )
{
	m_szName = szName;
	generalInit();
}

SceneNode::SceneNode()
{
	generalInit();
}

SceneNode::~SceneNode()
{
	 
}

void SceneNode::generalInit()
{
	static glm::mat4 identity = glm::mat4( 1.0f, 0.0f, 0.0f, 0.0f,
										   0.0f, 1.0f, 0.0f, 0.0f,
										   0.0f, 0.0f, 1.0f, 0.0f,
										   0.0f, 0.0f, 0.0f, 1.0f );

	m_clMatGlobalTransform = identity;
}

void SceneNode::render( ) const
{
	//render entities attatched to this node
	for( uint uIdx = 0; uIdx < m_vEntities.size(); ++uIdx )
	{
		m_vEntities[ uIdx ]->render();
	}

	//render all child-nodes recursively
	for(  uint uIdx = 0; uIdx < m_vChildNodes.size(); ++uIdx )
	{
		m_vChildNodes[ uIdx ]->render(); 
	}
}

void SceneNode::prepareRender()
{
	for( uint uIdx = 0; uIdx < m_vLights.size(); ++uIdx )
	{
		m_pParentScene->AddLightToRenderCache( m_vLights[ uIdx ], this );
	}

	for( uint uIdx = 0; uIdx < m_vEntities.size(); ++uIdx )
	{
		m_vEntities[ uIdx ]->prepareRender();
	}

	for(  uint uIdx = 0; uIdx < m_vChildNodes.size(); ++uIdx )
	{
		m_vChildNodes[ uIdx ]->prepareRender(); 
	}
}

const TransformSQT& SceneNode::getLocalTransform() const 
{
	return m_clLocalTransform;
}

const glm::mat4& SceneNode::getGlobalTransformMAT() const
{
	return m_clMatGlobalTransform;
}

void SceneNode::translate( const glm::vec3& rV3translation )
{
	m_clLocalTransform.m_v3Translation += rV3translation;
	updateTransformsFromParent();
}

void SceneNode::scale( const glm::vec3& rV3Scale )
{
	m_clLocalTransform.m_v3Scale *= rV3Scale;
	updateTransformsFromParent();
}

void SceneNode::rotate( const glm::quat& rQuatRotation )
{
	glm::quat normQuat = glm::normalize( rQuatRotation );
	glm::quat resultQuad = glm::cross( normQuat, m_clLocalTransform.m_quatRotation );
	m_clLocalTransform.m_quatRotation = resultQuad;
	updateTransformsFromParent();
}

void SceneNode::rotate( const float fAngle, const glm::vec3& rAxis )
{
	glm::quat quatRotation;
	quatRotation = QuaternionService::CreateRotationQuaternion( fAngle, rAxis );
	rotate( quatRotation );
}

void SceneNode::setTranslation( const glm::vec3& rV3translation )
{
	m_clLocalTransform.m_v3Translation = rV3translation;
	updateTransformsFromParent();
}

void SceneNode::setScale( const glm::vec3& rV3Scale )
{
	m_clLocalTransform.m_v3Scale = rV3Scale;
	updateTransformsFromParent();
}

void SceneNode::setRotation( const glm::quat& rQuatRotation )
{
	glm::quat normQuat;
	normQuat = glm::normalize( rQuatRotation ); 
	m_clLocalTransform.m_quatRotation = normQuat;
	updateTransformsFromParent();
}

void SceneNode::setRotation( const float fAngle, const glm::vec3& rAxis )
{
	glm::quat quatRotation = QuaternionService::CreateRotationQuaternion( fAngle, rAxis );
	setRotation( quatRotation ); 
}

void SceneNode::updateTransform( const glm::mat4& rParentGlobalMat )
{
	m_clMatGlobalTransform = rParentGlobalMat * m_clLocalTransform.getAsMat4(); //apply parent transformation first and then add local transform
	m_clGlobalTransformChanged.RaiseEvent( m_clMatGlobalTransform );

	updateChildrenTransforms();
}

void SceneNode::setTransform( const TransformSQT& rClTransform )
{
	m_clLocalTransform = rClTransform;
	updateTransformsFromParent();		
}

void SceneNode::updateTransformsFromParent()
{
	if( m_pParentNode )
	{
		m_clMatGlobalTransform = m_pParentNode->m_clMatGlobalTransform * m_clLocalTransform.getAsMat4();
	}

	else
	{
		m_clMatGlobalTransform = m_clLocalTransform.getAsMat4();
	}

	//Since matrices do not have a compare-functionality and since storing yet another matrix is a waste of memory, just raise the event all the time for now...
	m_clGlobalTransformChanged.RaiseEvent( m_clMatGlobalTransform );

	updateChildrenTransforms();
}

void SceneNode::updateChildrenTransforms()
{
	for( uint uIdx = 0; uIdx < m_vChildNodes.size(); ++uIdx )
	{
		m_vChildNodes[ uIdx ]->updateTransform( m_clMatGlobalTransform ); 
	}
}

SceneNode* SceneNode::createChildSceneNode( const std::string& szName )
{
	NodeRegistry& registry = NodeRegistry::getInstance();

	if( registry.isObjectRegistered( szName ) )
	{
		SceneNode* pNode = registry.getObject( szName );
		if( pNode->getParent() == this ) //already a child of this node!
		{
			return pNode;
		}

		else //node exists already but not as a child of this node
		{
			return NULL;
		}
	}

	else //node non-existent yet
	{
		SceneNode* pChildNode = new SceneNode( szName );
		AppendChildSceneNode( pChildNode );
		return pChildNode;
	}
}

bool SceneNode::AppendChildSceneNode( SceneNode* pNode )
{
	if( !pNode )
		return false;

	if( pNode->m_pParentNode && pNode->m_pParentNode == this )
		return false;

	if( pNode->m_pParentNode )
		pNode->m_pParentNode->removeChildSceneNode( pNode );

	NodeRegistry& registry = NodeRegistry::getInstance();

	if( !registry.isObjectRegistered( pNode->getName() ) )
		registry.registerObject( pNode->getName(), pNode );

	pNode->m_pParentNode = this;
	pNode->SetScene( m_pParentScene );
	m_vChildNodes.push_back( pNode );
	updateChildrenTransforms();

	return true;
}

bool SceneNode::removeChildSceneNode( const std::string& szName )
{
	NodeRegistry& registry = NodeRegistry::getInstance();

	if( registry.isObjectRegistered( szName ) )
	{
		SceneNode* pNode = registry.getObject( szName );
		return removeChildSceneNode( pNode );
	}

	return false;
}

bool SceneNode::removeChildSceneNode( SceneNode* pNode )
{
	if( !pNode )
	{
		return false;
	}

	if( pNode->getParent() == this )
	{
		std::vector<SceneNode*>::iterator iter = std::find( m_vChildNodes.begin(), m_vChildNodes.end(), pNode );
		if( iter != m_vChildNodes.end() )
		{
			m_vChildNodes.erase( iter );
			pNode->m_pParentNode = NULL;
			return true;
		}
	}

	return false;
}

bool SceneNode::destroyChildSceneNode( const std::string& szName )
{
	NodeRegistry& registry = NodeRegistry::getInstance();

	if( registry.isObjectRegistered( szName ) )
	{
		SceneNode* pNode = registry.getObject( szName );
		return destroyChildSceneNode( pNode );
	}

	return false;
}

bool SceneNode::destroyChildSceneNode( SceneNode* pNode )
{
	if( !pNode )
	{
		return false;
	}

	if( pNode->getParent() == this )
	{
		std::vector<SceneNode*>::iterator iter = std::find( m_vChildNodes.begin(), m_vChildNodes.end(), pNode );
		if( iter != m_vChildNodes.end() )
		{
			m_vChildNodes.erase( iter );
			//recursively destroy node and its child-nodes
			pNode->destroyNode();
			SAFE_DELETE( pNode );
			return true;
		}
	}

	return false;
}

void SceneNode::destroyNode()
{
	//destroy attatched entities
	for( uint uIdx = 0; uIdx < m_vEntities.size(); ++uIdx )
	{
		BaseRenderableObject* pEntity = m_vEntities[ uIdx ];
		removeEntity( pEntity, true );
	}

	m_vEntities.clear();

	//destroy child-nodes
	for( uint uIdx = 0; uIdx < m_vChildNodes.size(); ++uIdx )
	{
		SceneNode* pNode = m_vChildNodes[ uIdx ];
		removeChildSceneNode( pNode );
	}

	m_vChildNodes.clear();
}

SceneNode* SceneNode::getParent()
{
	return m_pParentNode;
}

bool SceneNode::attatchEntity( BaseRenderableObject* const _pNewEntity )
{
	if( std::find( m_vEntities.begin(), m_vEntities.end(), _pNewEntity ) != m_vEntities.end() )
	{
		return false;
	}
	
	m_vEntities.push_back( _pNewEntity );
	_pNewEntity->attatchToNode( this , 0 );
	updateTransformsFromParent();
	return true;
}

bool SceneNode::AttatchLight( Light* pLight )
{
	if( std::find( m_vLights.begin(), m_vLights.end(), pLight ) != m_vLights.end() )
	{
		return false;
	}

	m_vLights.push_back( pLight );
	return true;
}


bool SceneNode::removeEntity( const BaseRenderableObject* _pRemoveEntity, bool bDeleteFromProgram )
{
	if( !_pRemoveEntity )
	{
		return false;
	}
	
	std::vector<BaseRenderableObject*>::iterator eraseIter = std::find( m_vEntities.begin(), m_vEntities.end(), _pRemoveEntity );
	if( eraseIter == m_vEntities.end() )
	{
		return false;
	}
	
	//let the entity destroy and detatch itself
	(*eraseIter)->destroyEntity();

	if( bDeleteFromProgram )
	{
		SAFE_DELETE( _pRemoveEntity );
	}

	m_vEntities.erase( eraseIter );
	
	return true;
}

bool SceneNode::removeEntity( const String& name, bool bDeleteFromProgram )
{
	return true;
}

BaseRenderableObject* SceneNode::getEntity( const String& name )
{
	return NULL;
}

const String SceneNode::getName() const
{
	return m_szName;
}

