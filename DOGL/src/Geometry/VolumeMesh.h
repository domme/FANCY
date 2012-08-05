#ifndef VOLUMEMESH_H
#define VOLUMEMESH_H

#include "../includes.h"

#include "../Rendering/GLTexture.h"
#include "../Rendering/Materials/VolumeMaterial.h"
#include "../Rendering/Materials/MAT_VolCube_Rasterize.h"
#include "../Math/TransformSQT.h"
#include "BaseGeometryObject.h"
#include "Mesh.h"

class Renderer;
class Material;
class Camera;


class DLLEXPORT VolumeMesh : public BaseGeometryObject
{
	friend class GLRenderer;
	friend class GLDeferredRenderer;
	friend class GLSLProgram;

public:
	VolumeMesh() :	m_pVolumeMaterial( NULL )
	{}

	VolumeMesh( VolumeMesh& other );

	VolumeMesh( VolumeMesh&& other );
	VolumeMesh& operator= ( VolumeMesh&& other );
	
	~VolumeMesh();

	void							CloneInto( VolumeMesh& other );
	void							Create( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension, VolumeMaterial* pMaterial, const String& szTransferFuktionPath );
	void							Init( const std::vector<glm::vec3>& vertexPositions );
	
	VolumeMaterial*					GetMaterial() { return m_pVolumeMaterial; }
	const VolumeMaterial*			GetMaterial() const { return m_pVolumeMaterial; }
	void							SetMaterial( VolumeMaterial* const pNewMat ) { if( pNewMat ) m_pVolumeMaterial = pNewMat; }

	const GLTexture*				GetVolumeTexture() const { return &m_clVolumeTexture; }
	const GLTexture*				GetTransferFktTexture() const { return &m_clTransferfunktionTexture; }
	void							SetVolumeTexure( GLuint uTex );
	void							SetVolumeTexure( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension );
	void							SetTransferFunktionTexture( GLuint uTex );
	void							SetTransferFunktionTexture( const String& szPath );

	static Mesh*				GetVolumeBoundingMesh() { return &m_clVolumeBoundingCube; }
	static Material*			GetVolumeBoundingRasterizeMaterial() { return m_pMatRasterize; }

private:
	VolumeMaterial*			m_pVolumeMaterial;
	GLTexture				m_clVolumeTexture;
	GLTexture				m_clTransferfunktionTexture;

protected:
	static Mesh						m_clVolumeBoundingCube;
	static Material*				m_pMatRasterize;
	
	void					initMeshAndMaterial();

};

#endif