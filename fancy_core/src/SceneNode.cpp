#include "SceneNode.h"
#include "SceneNodeComponentFactory.h"
#include "SceneRenderDescription.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  Transform::Transform() :
    m_local(1.0f),
    m_cachedWorld(1.0f)
  {

  }
//---------------------------------------------------------------------------//
  Transform::~Transform()
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  SceneNode::SceneNode() :
    m_pParent(nullptr)
  {

  }
//---------------------------------------------------------------------------//
  SceneNode::~SceneNode()
  {
    
  }
//---------------------------------------------------------------------------//
  void SceneNode::parentNodeToNode(std::shared_ptr<SceneNode> pChild, std::shared_ptr<SceneNode> pParent)
  {
    SceneNode::parentNodeToNode(pChild, pParent.get());
  }
//---------------------------------------------------------------------------//
  void SceneNode::unparentNode(std::shared_ptr<SceneNode> pChild)
  {
    SceneNode::unparentNode(pChild.get());
  }
//---------------------------------------------------------------------------//
  void SceneNode::parentNodeToNode(std::shared_ptr<SceneNode> pChild, SceneNode* pParent)
  {
    if (pChild->hasParent())
    {
      SceneNode::unparentNode(pChild);
    }

    pParent->m_vpChildren.push_back(pChild);
    pChild->m_pParent = pParent;
  }
  //---------------------------------------------------------------------------//
  void SceneNode::unparentNode(SceneNode* pChild)
  {
    if (!pChild->hasParent())
    {
      return;
    }

    SceneNode* pParent = pChild->m_pParent;

    std::vector<std::shared_ptr<SceneNode>>::iterator it =
      std::find_if(pParent->m_vpChildren.begin(), pParent->m_vpChildren.end(), 
      [pChild] (const std::shared_ptr<SceneNode>& it) { return it.get() == pChild; } );

    ASSERT_M(it != pParent->m_vpChildren.end(), "Invalid parent-child relationship");

    pChild->m_pParent = nullptr;
    pParent->m_vpChildren.erase(it);
  }
//---------------------------------------------------------------------------//
  void SceneNode::update()
  {
    for (uint i = 0u; i < m_vpChildren.size(); ++i)
    {
      Transform& childTransform = m_vpChildren[i]->getTransform();
      childTransform.m_cachedWorld = childTransform.m_local * getTransform().m_cachedWorld;

      m_vpChildren[i]->update();
    }

    for (uint i = 0u; i < m_vpComponents.size(); ++i)
    {
      m_vpComponents[i]->update();
    }
  } 
//---------------------------------------------------------------------------//
  void SceneNode::gatherRenderItems(SceneRenderDescription* pRenderDesc)
  {
    for (uint i = 0u; i < m_vpChildren.size(); ++i)
    {
      m_vpChildren[i]->gatherRenderItems(pRenderDesc);
    }

    for (uint i = 0u; i < m_vpComponents.size(); ++i)
    {
      m_vpComponents[i]->gatherRenderItems(pRenderDesc);
    }
  }
//---------------------------------------------------------------------------//
  SceneNodeComponentWeakPtr SceneNode::getComponentPtr( const ObjectName& typeName )
  {
    for (uint i = 0; i < m_vpComponents.size(); ++i)
    {
      if (m_vpComponents[i]->getTypeName() == typeName)
      {
        return m_vpComponents[i];
      }
    }

    return SceneNodeComponentWeakPtr();  // nullptr
  }
//---------------------------------------------------------------------------//
  ModelComponentWeakPtr SceneNode::getModelComponentPtr()
  {
    return std::static_pointer_cast<ModelComponent>(getComponentPtr(_N(Model)).lock());
  }
//---------------------------------------------------------------------------//
  CameraComponentWeakPtr SceneNode::getCameraComponentPtr()
  {
    return std::static_pointer_cast<CameraComponent>(getComponentPtr(_N(Camera)).lock());
  }
//---------------------------------------------------------------------------//
  SceneNodeComponent* SceneNode::getComponent( const ObjectName& typeName )
  {
    for (uint i = 0; i < m_vpComponents.size(); ++i)
    {
      if (m_vpComponents[i]->getTypeName() == typeName)
      {
        return m_vpComponents[i].get();
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SceneNodeComponent* SceneNode::createComponent( const ObjectName& typeName )
  {
    if (getComponent(typeName))
    {
      return getComponent(typeName);
    }

    SceneNodeComponentFactory::CreateFunction createFunc = SceneNodeComponentFactory::getFactoryMethod(typeName);
    ASSERT_M(createFunc != nullptr, String("No factory registered for typename ") + typeName.toString());

    SceneNodeComponentPtr componentPtr = createFunc(const_cast<SceneNode*>(this));

    m_vpComponents.push_back(componentPtr);
    onComponentAdded(componentPtr);

    return componentPtr.get();
  }
//---------------------------------------------------------------------------//
  void SceneNode::removeComponent(const ObjectName& typeName)
  {
   std::vector<SceneNodeComponentPtr>::iterator itFound = m_vpComponents.end();
    for (std::vector<SceneNodeComponentPtr>::iterator it = m_vpComponents.begin();
      it != m_vpComponents.end(); ++it)
    {
      if ((*it)->getTypeName() == typeName)
      {
        itFound = it;
        break;
      }
    }

    if (itFound == m_vpComponents.end())
    {
      return;
    }
    
    onComponentRemoved(*itFound);

    m_vpComponents.erase(itFound);
  }
//---------------------------------------------------------------------------//
  void SceneNode::onComponentAdded(const SceneNodeComponentPtr& pComponent )
  {
    ObjectName typeName = pComponent->getTypeName();

    if (typeName == _N(Model)) {
      m_pModelComponent = static_cast<ModelComponent*>(pComponent.get());
    } else if (typeName == _N(Camera)) {
      m_pCameraComponent = static_cast<CameraComponent*>(pComponent.get());
    }
  }
//---------------------------------------------------------------------------//
  void SceneNode::onComponentRemoved(const SceneNodeComponentPtr& pComponent )
  {
    ObjectName typeName = pComponent->getTypeName();

    if (typeName == _N(Model)) {
      m_pModelComponent = nullptr;
    } else if (typeName == _N(Camera)) {
      m_pCameraComponent = nullptr;
    }
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#pragma region Old SceneNode code

/*
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
    return pNode; //Node already exists. (May not be a child of this node though!)
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
*/


#pragma endregion Old SceneNode code