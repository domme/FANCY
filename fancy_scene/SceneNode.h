#ifndef INCLUDE_SCENENODE_H
#define INCLUDE_SCENENODE_H

#include "FancyCorePrerequisites.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "ObjectName.h"
#include "Serializable.h"

namespace Fancy {
  namespace IO {
    class Serializer;
  }
}

namespace Fancy { namespace Scene { 
//---------------------------------------------------------------------------//
  class Scene;
//---------------------------------------------------------------------------//
  class Transform
  {
  private:
    friend class SceneNode;

    public:
      Transform();
      ~Transform();

      const glm::mat4& getCachedWorld() const {return m_cachedWorld;}
      glm::mat4 getLocalAsMat() const;

      //void rotate(const glm::quat& _quat);
      //void rotateLocal(const glm::quat& _quat);
      void rotate(const glm::vec3& _axis, float _degree);
      void rotateLocal(const glm::vec3& _axis, float _degree);
      void translate(const glm::vec3& _translation);
      void translateLocal(const glm::vec3& _translation);
      void scale(const glm::vec3& _scale);
      void scaleLocal(const glm::vec3& _scale);

      const glm::quat& getRotationLocal() const {return m_localRotation;}
      const glm::vec3& getPositionLocal() const {return m_localPosition;}
      const glm::vec3& getScaleLocal() const {return m_localScale;}
      void setRotationLocal(const glm::quat& _rot) {m_localRotation = _rot; m_dirty = true; }
      void setPositionLocal(const glm::vec3& _pos) {m_localPosition = _pos; m_dirty = true; }
      void setScaleLocal(const glm::vec3& _scale) {m_localScale = _scale; m_dirty = true; }

      glm::vec3 getPosition() const { return static_cast<glm::vec3>(m_cachedWorld[3]); }
      glm::vec3 right() const {return glm::normalize(static_cast<glm::vec3>(m_cachedWorld[0]));}
      glm::vec3 up() const {return glm::normalize(static_cast<glm::vec3>(m_cachedWorld[1]));}
      glm::vec3 forward() const {return glm::normalize(static_cast<glm::vec3>(m_cachedWorld[2]));}

    private:
      bool m_dirty;

      glm::quat m_localRotation;
      glm::vec3 m_localPosition;
      glm::vec3 m_localScale;

      glm::mat4 m_cachedWorld;
      glm::mat4 m_parentWorld;
  };
//---------------------------------------------------------------------------//
  class SceneRenderDescription;
//---------------------------------------------------------------------------//
  class SceneNode
  {
    public: 
      SERIALIZABLE(SceneNode)

      SceneNode(Scene* aScene);
      ~SceneNode();

      static ObjectName getTypeName() { return _N(SceneNode); }
      void Serialize(IO::Serializer* aSerializer);
      uint64 GetHash() const { return 0u; }

      static void AddChildNode(std::shared_ptr<SceneNode> pChild, std::shared_ptr<SceneNode> pParent);
      static void AddChildNode(std::shared_ptr<SceneNode> pChild, SceneNode* pParent);
      static void RemoveChildNode(std::shared_ptr<SceneNode> pChild);
      static void RemoveChildNode(SceneNode* pChild);

      void startup();
      void update(float _dt, const std::function<void(SceneNodeComponent*)>& aComponentCallback);

      Scene* GetScene() const { return myScene; }

      SceneNodeComponent* addOrRetrieveComponent(const ObjectName& typeName);
      SceneNodeComponentPtr addOrRetrieveComponentPtr(const ObjectName& typeName);
      void removeComponent(const ObjectName& typeName);
      SceneNodeComponentPtr getComponentPtr(const ObjectName& typeName);
      SceneNodeComponent* getComponent(const ObjectName& typeName);

      SceneNode* createChildNode(const ObjectName& _name = ObjectName::blank);

      Transform& getTransform() {return m_transform;}
      const Transform& getTransform() const {return m_transform;}
      bool hasParent() const {return m_pParent != nullptr;}

      const ObjectName& getName() const {return m_name;}
      void setName(const ObjectName& _name) {m_name = _name;}

  private:
      std::vector<SceneNodeComponentPtr> m_vpComponents;
      std::vector<std::shared_ptr<SceneNode>> m_vpChildren;

      SceneNode* m_pParent;
      Transform m_transform;
      ObjectName m_name;
      Scene* myScene;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNode)
//---------------------------------------------------------------------------//
} // End of namespace Scene
//---------------------------------------------------------------------------//
} // end of namespace Fancy

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

class  SceneNode
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