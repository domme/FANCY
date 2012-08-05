#ifndef MESH_H
#define MESH_H

#include "../includes.h"

#include "../Rendering/Materials/Material.h"
#include "../Rendering/Shader.h"
#include "VertexDeclarations.h"
#include "../Math/TransformSQT.h"
#include "BaseGeometryObject.h"

class Renderer;
class Material;
class Camera;

class DLLEXPORT Mesh : public BaseGeometryObject
{
	friend class OBJimporter;
	friend class GLRenderer;
	friend class GLDeferredRenderer;
	friend class GLSLProgram;

public:
	Mesh() :	m_pMaterial( NULL ),
				m_pVertexInfo( NULL )
	{}

	Mesh( Mesh& other );

	Mesh( Mesh&& other );
	Mesh& operator= ( Mesh&& other );

	~Mesh();

	void							CloneInto( Mesh& other );
	void							Create( VertexDeclaration* pVertexInfo, Material* pMaterial, const std::vector<glm::vec3>& vertexPositions );
	void							Init( const std::vector<glm::vec3>& vertexPositions );
	//void							Init( const Vertex::VertexType* pVertices, uint32 uNumVertices, const uint32* puIndices, uint32 uNumIndices, Material* pMaterial );
	Material*						GetMaterial() { return m_pMaterial; }
	const Material*					GetMaterial() const { return m_pMaterial; }
	void							SetMaterial( Material* const pNewMat ) { if( pNewMat ) m_pMaterial = pNewMat; }

	void							SetVertexInfo( VertexDeclaration* pNewVertexInfo );
	const VertexDeclaration*		GetVertexInfo() const { return m_pVertexInfo; }


private:
	Material*				m_pMaterial;
	
	VertexDeclaration*		m_pVertexInfo;
};

#endif