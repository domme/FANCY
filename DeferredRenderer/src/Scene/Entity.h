#ifndef ENTITY_H
#define ENTITY_H

#include <Includes.h>
#include <Rendering/Materials/Material.h>
#include <Scene/BoundingSphere.h>

#include "BaseRenderableObject.h"

class SceneNode;
class SceneManager;

class  Entity : public BaseRenderableObject
{
	friend class SceneManager;

public:
	bool							SetMesh( std::unique_ptr<Mesh> pMesh );
	bool							SetMesh( const String& szMeshFilename );
									   
	bool							HasMesh() { return m_pMesh != NULL; }
	std::unique_ptr<Mesh>&			GetMesh() { return m_pMesh; }
	const std::unique_ptr<Mesh>&	GetMesh() const { return m_pMesh; }
	void							updateSceneBounds();
	
	virtual const BoundingSphere&	getBoundingSphere();
	virtual void					destroyEntity();
	virtual void					render();
	virtual void					prepareRender();
	virtual void					onAttatchedToNode();

private:
	Entity();
	~Entity();
	
	std::unique_ptr<Mesh>			m_pMesh;

};

#endif