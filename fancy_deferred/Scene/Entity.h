#ifndef ENTITY_H
#define ENTITY_H

#include <Includes.h>
#include <Rendering/Materials/Material.h>
#include <Scene/BoundingSphere.h>

#include "BaseRenderableObject.h"

class SceneNode;
class SceneManager;

class DLLEXPORT Entity : public BaseRenderableObject
{
	friend class SceneManager;

public:
	bool							SetMesh( Mesh* pMesh );
	bool							SetMesh( const String& szMeshFilename );
									   
	bool							HasMesh() { return m_pMesh != NULL; }
	Mesh*							GetMesh() { return m_pMesh; }
	const Mesh*						GetMesh() const { return m_pMesh; }
	void							updateSceneBounds();
	
	virtual const BoundingSphere&	getBoundingSphere();
	virtual void					destroyEntity();
	virtual void					render();
	virtual void					prepareRender();
	virtual void					onAttatchedToNode();

private:
	Entity();
	~Entity();
	
	Mesh*							m_pMesh;

};

#endif