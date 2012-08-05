#ifndef VOLUMEENTITY_H
#define VOLUMEENTITY_H

#include "BaseRenderableObject.h"
#include "../includes.h"
#include "../Rendering/Materials/VolumeMaterial.h"
#include "BoundingSphere.h"
#include "AABoundingBox.h"
#include "../Rendering/GLTexture.h"
#include "../Geometry/VolumeMesh.h"


class SceneNode;
class SceneManager;

class DLLEXPORT VolumeEntity : public BaseRenderableObject
{
	friend class SceneManager;

public:
	bool							SetVolumeMesh( std::unique_ptr<VolumeMesh> pMesh );
	bool							SetVolumeMesh( const String& szBaseTexPath, uint uTexStartIndex, uint uTexEndIndex, const String& szExtension, VolumeMaterial* pVolumeMaterial, const String& szTransferFuktionPath );

	bool							HasMesh() { return m_pVolumeMesh != NULL; }
	std::unique_ptr<VolumeMesh>&	GetVolumeMesh() { return m_pVolumeMesh; }
	const std::unique_ptr<VolumeMesh>&	GetVolumeMesh() const { return m_pVolumeMesh; }
	void							updateSceneBounds();

	virtual const BoundingSphere&	getBoundingSphere();	  
	virtual void					destroyEntity();
	virtual void					render();
	virtual void					prepareRender();
	virtual void					onAttatchedToNode();

private:
	VolumeEntity();
	~VolumeEntity();

	std::unique_ptr<VolumeMesh>		m_pVolumeMesh;

	

};

#endif