
#ifndef BASERENDERABLEOBJECT_H
#define BASERENDERABLEOBJECT_H

#include "../includes.h"


class AABoundingBox;
class BoundingSphere;
class SceneNode;

class DLLEXPORT BaseRenderableObject
{
public:
	BaseRenderableObject();

	enum EAttatchmentFlags
	{
		ATTATCH_DEFAULT = 0x0000,
		ATTATCH_FORCE	= 0x0001,
	};

	virtual	void			onAttatchObject( const BaseRenderableObject* newObject );
	virtual void			onDeleteObject( const BaseRenderableObject* deletedObject );
	virtual void			onRequestUpdate( void );
	virtual void			onTransformChanged( const glm::mat4& newTransform );
	virtual void			onAttatchedToNode();
	bool					isEnabled() const;
	void					setEnabled( bool bEnabled );
	bool					isVisible() const;
	void					setVisible( bool bVisible );
	bool					isAttatched() const;
	SceneNode*				getNode() const;
	bool					attatchToNode( SceneNode* const pNode, uint16 u16AttatchmentFlags = 0x0000 );
	void					detatch();
	virtual const BoundingSphere&	getBoundingSphere() = 0;
	virtual void			destroyEntity() = 0;
	virtual void			render() = 0;
	virtual void			prepareRender() = 0;

	virtual ~BaseRenderableObject();
	
protected:
	
	bool			m_bVisible;
	bool			m_bEnabled;
	SceneNode*		m_pNode;

	int				m_iTransformChangedListenerID;
};




#endif