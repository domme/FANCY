
#ifndef SCENENODE_H
#define	SCENENODE_H

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

	bool										AppendChildSceneNode( SceneNode* pNode );
	SceneNode*									createChildSceneNode( const std::string& szName ); 
	SceneNode*									getParent();
	bool										destroyChildSceneNode( const std::string& szName );
	bool										destroyChildSceneNode( SceneNode* pNode );
	bool										removeChildSceneNode( const std::string& szName );
	bool										removeChildSceneNode( SceneNode* pNode );
	bool										attatchEntity( BaseRenderableObject* const _pNewEntity );
	bool										removeEntity( const BaseRenderableObject* _pRemoveEntity, bool bDeleteFromProgram = false );
	bool										removeEntity( const String& name, bool bDeleteFromProgram = false );
	BaseRenderableObject*						getEntity( const String& name );
	void										destroyNode();
	const String								getName() const;
	const TransformSQT&							getLocalTransform() const;
	const glm::mat4&							getGlobalTransformMAT() const;
	void										prepareRender();
	void										render() const;

	EventHandler1Param<const glm::mat4&>		m_clGlobalTransformChanged;

private:
	SceneNode( const String& name );
	SceneNode();


	void										generalInit();
	void										updateChildrenTransforms();

	void										updateTransformsFromParent();

	std::vector<BaseRenderableObject*>			m_vEntities;
	std::vector<SceneNode*>						m_vChildNodes;
	TransformSQT								m_clLocalTransform;
	glm::mat4									m_clMatGlobalTransform;
	String										m_szName;
	SceneNode*									m_pParentNode;



};




#endif
