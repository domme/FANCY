#ifndef INCLUDE_SCENENODE_H
#define INCLUDE_SCENENODE_H

#include "FancyCorePrerequisites.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "ObjectName.h"

namespace Fancy { namespace Scene { 
//---------------------------------------------------------------------------//
  class DLLEXPORT Transform
  {
    friend class SceneNode;

    public:
      Transform();
      ~Transform();

      const glm::mat4& getLocal() const {return m_local;}
      const glm::mat4& getCachedWorld() const {return m_cachedWorld;}

      void setLocal(const glm::mat4& _val) {m_local = _val; m_dirty = true;}
      
    private:
      bool m_dirty;
      glm::mat4 m_local;
      glm::mat4 m_cachedWorld;
  };
//---------------------------------------------------------------------------//
  class SceneRenderDescription;
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNode
  {
    public: 
      SceneNode();
      ~SceneNode();

      static void parentNodeToNode(std::shared_ptr<SceneNode> pChild, std::shared_ptr<SceneNode> pParent);
      static void parentNodeToNode(std::shared_ptr<SceneNode> pChild, SceneNode* pParent);
      static void unparentNode(std::shared_ptr<SceneNode> pChild);
      static void unparentNode(SceneNode* pChild);

      void update();
      void gatherRenderItems(SceneRenderDescription* pRenderDesc);

      SceneNodeComponent* addOrRetrieveComponent(const ObjectName& typeName);
      void removeComponent(const ObjectName& typeName);
      SceneNodeComponentWeakPtr getComponentPtr(const ObjectName& typeName);
      SceneNodeComponent* getComponent(const ObjectName& typeName);

      SceneNode* createChildNode(const ObjectName& _name = ObjectName::blank);

      CameraComponentWeakPtr getCameraComponentPtr();
      CameraComponent* getCameraComponent() {return m_pCameraComponent;}
      ModelComponentWeakPtr getModelComponentPtr();
      ModelComponent* getModelComponent() {return m_pModelComponent;}

      Transform& getTransform() {return m_transform;}
      const Transform& getTransform() const {return m_transform;}
      bool hasParent() const {return m_pParent != nullptr;}

      const ObjectName& getName() const {return m_name;}
      void setName(const ObjectName& _name) {m_name = _name;}

  private:
      void onComponentAdded(const SceneNodeComponentPtr& pComponent);
      void onComponentRemoved(const SceneNodeComponentPtr& pComponent);

      std::vector<SceneNodeComponentPtr> m_vpComponents;
      std::vector<std::shared_ptr<SceneNode>> m_vpChildren;
      SceneNode* m_pParent;
      Transform m_transform;
      ObjectName m_name;

      // Cached components:
      CameraComponent* m_pCameraComponent;
      ModelComponent* m_pModelComponent;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNode)
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#pragma region Old SceneNode code

/*
#ifndef SCENENODE_H
#define	SCENENODE_H

#include <Light/Light.h>
#include <Events/Events.h>
#include <includes.h>

#include "BaseRenderableObject.h"
#include "../Math/TransformSQT.h"


class SceneManager;

class DLLEXPORT  SceneNode
{
  friend class SceneManager;
  friend class SceneLoader;	

public:
  virtual ~SceneNode();

  void										translate( const glm::vec3& rV3translation );
  void										scale( const glm::vec3& rV3Scale );
  void										rotate( const glm::quat& rQuatRotation );
  void										rotate( const float fAngle, const glm::vec3& rAxis );

  void										setTranslation( const glm::vec3& rV3translation );
  void										setScale( const glm::vec3& rV3Scale );
  void										setRotation( const glm::quat& rQuatRotation );
  void										setRotation( const float fAngle, const glm::vec3& rAxis );
  void										setTransform( const TransformSQT& rClTransform );
  void										setTransform( const glm::mat4& matTransform ) { setTransform( TransformSQT( matTransform ) ); }
  void										updateTransform( const glm::mat4& rParentGlobalMat );
  void										SetScene( SceneManager* pScene ) {m_pParentScene = pScene; }

  bool										AppendChildSceneNode( SceneNode* pNode );
  SceneNode*									createChildSceneNode( const std::string& szName ); 
  SceneNode*									getParent();
  bool										destroyChildSceneNode( const std::string& szName );
  bool										destroyChildSceneNode( SceneNode* pNode );
  bool										removeChildSceneNode( const std::string& szName );
  bool										removeChildSceneNode( SceneNode* pNode );
  bool										attatchEntity( BaseRenderableObject* const _pNewEntity );
  bool										AttatchLight( Light* pLight );
  bool										removeEntity( const BaseRenderableObject* _pRemoveEntity, bool bDeleteFromProgram = false );
  bool										removeEntity( const String& name, bool bDeleteFromProgram = false );
  BaseRenderableObject*						getEntity( const String& name );
  void										destroyNode();
  const String								getName() const;
  const TransformSQT&							getLocalTransform() const;
  const glm::mat4&							getGlobalTransformMAT() const;
  void										prepareRender();
  void										render() const;

  Delegate1Param<glm::mat4>					m_clGlobalTransformChanged;

private:
  SceneNode( const String& name );
  SceneNode();


  void										generalInit();
  void										updateChildrenTransforms();

  void										updateTransformsFromParent();

  std::vector<BaseRenderableObject*>			m_vEntities;
  std::vector<Light*>							m_vLights;
  std::vector<SceneNode*>						m_vChildNodes;
  TransformSQT								m_clLocalTransform;
  glm::mat4									m_clMatGlobalTransform;
  String										m_szName;
  SceneNode*									m_pParentNode;
  SceneManager*								m_pParentScene;



};




#endif
*/
#pragma endregion Old SceneNode code


#endif  // INCLUDE_SCENENODE_H