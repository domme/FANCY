
#include <Includes.h>
#include <Scene/AABoundingBox.h>

#include "BaseRenderableObject.h"

#include "SceneNode.h"


BaseRenderableObject::BaseRenderableObject() :
m_pNode( 0 ),
m_bEnabled( true ),
m_bVisible( true )
{

}


BaseRenderableObject::~BaseRenderableObject()
{
	
}

void BaseRenderableObject::onAttatchObject( const BaseRenderableObject* newObject )
{
	//Dummy in base class: do nothing
}

void BaseRenderableObject::onDeleteObject( const BaseRenderableObject* deletedObject )
{
	//Dummy in base class: do nothing
}

void BaseRenderableObject::onRequestUpdate( void )
{
	//Dummy in base class: do nothing
}

void BaseRenderableObject::onTransformChanged( const glm::mat4& newTransform )
{
	//Dummy in base class: do nothing
}

void BaseRenderableObject::onAttatchedToNode()
{
	//Dummy in base class: do nothing
}

void BaseRenderableObject::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
}

bool BaseRenderableObject::isEnabled() const
{
	return m_bEnabled;
}

bool BaseRenderableObject::isVisible() const
{
	return m_bVisible;
}

void BaseRenderableObject::setVisible( bool bVisible )
{
	m_bVisible = bVisible;
}

bool BaseRenderableObject::isAttatched() const
{
	return m_pNode != NULL;
}

SceneNode* BaseRenderableObject::getNode() const
{
	return m_pNode;
}

bool BaseRenderableObject::attatchToNode( SceneNode* const pNode, uint16 u16AttatchmentFlags /*= 0x0000*/ )
{
	if( !pNode )
	{
		return false;
	}
	
	if( isAttatched() && !( u16AttatchmentFlags & ATTATCH_FORCE ) )
	{
		return false;
	}
	
	//this method assumes that attatch is called from the attatchment-method
	//within the scene node so that calling the attatchment calculus of the node from
	//here is obsolete...
	m_pNode = pNode;

	m_iTransformChangedListenerID = pNode->m_clGlobalTransformChanged.registerListener<BaseRenderableObject>(this, &BaseRenderableObject::onTransformChanged );

	onAttatchedToNode();
	return true;
}

void BaseRenderableObject::detatch()
{
	if( !isAttatched() )
	{
		return;
	}
	
	m_pNode->m_clGlobalTransformChanged.unregisterListener( m_iTransformChangedListenerID );
	m_pNode = NULL;
}

