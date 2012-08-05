#include "../includes.h"

#include <assert.h>
#include <iostream>
#include <algorithm>

#include "VolumeMesh.h"
#include "../Rendering/GLRenderer.h"
#include "../Engine.h"

#include "../Rendering/Managers/GLResourceManager.h"


Mesh VolumeMesh::m_clVolumeBoundingCube;
Material* VolumeMesh::m_pMatRasterize;

VolumeMesh::VolumeMesh( VolumeMesh& other )  :
m_pVolumeMaterial( NULL )
{
	if( other.m_pVolumeMaterial )
		m_pVolumeMaterial = other.m_pVolumeMaterial->CloneVolumeMat();

	m_clAABB = other.m_clAABB;
	m_clBoundingSphere = other.m_clBoundingSphere;
	m_szName = other.m_szName;
}

VolumeMesh::VolumeMesh( VolumeMesh&& other ) 
{
	*this = std::move( other );
}

VolumeMesh& VolumeMesh::operator= ( VolumeMesh&& other )
{
	if( this == &other )
		return *this;

	SAFE_DELETE( m_pVolumeMaterial );

	m_pVolumeMaterial = other.m_pVolumeMaterial;
	
	m_clAABB = other.m_clAABB;
	m_clBoundingSphere = other.m_clBoundingSphere;
	m_szName = std::move( other.m_szName );

	other.m_pVolumeMaterial = NULL;

	return *this;
}

VolumeMesh::~VolumeMesh()
{
	SAFE_DELETE( m_pVolumeMaterial );
}

void VolumeMesh::CloneInto( VolumeMesh& other )
{

}

void VolumeMesh::Create( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension, VolumeMaterial* pMaterial, const String& szTransferFuktionPath )
{
	m_pVolumeMaterial = pMaterial;

	SetVolumeTexure( szBaseTexPath, uStartIndex, uEndIndex, szExtension );
	SetTransferFunktionTexture( szTransferFuktionPath );

	//TODO: Improve these dummy-positions by using the actual 3D-Texture dimensions!
	std::vector<glm::vec3> vPositions;
	vPositions.push_back( glm::vec3( -1.0, -1.0, -1.0 ) ); //0
	vPositions.push_back( glm::vec3( 1.0, -1.0, -1.0 ) );	//1
	vPositions.push_back( glm::vec3( 1.0, 1.0, -1.0 ) );	//2
	vPositions.push_back( glm::vec3( -1.0, 1.0, -1.0 ) );	//3
	vPositions.push_back( glm::vec3( 1.0, -1.0, 1.0 ) );	//4
	vPositions.push_back( glm::vec3( 1.0, 1.0, 1.0 ) );		//5
	vPositions.push_back( glm::vec3( -1.0, 1.0, 1.0 ) );	//6
	vPositions.push_back( glm::vec3( -1.0, -1.0, 1.0 ) );	//7

	Init( vPositions );
}


void VolumeMesh::Init( const std::vector<glm::vec3>& vertexPositions )
{	
	GLRenderer::GetInstance().InitVolumeMesh( this );

	initBoundingGeometry( vertexPositions );

	RUN_ONLY_ONCE_STATIC( initMeshAndMaterial() );
}

void VolumeMesh::SetVolumeTexure( GLuint uTex )
{
	m_clVolumeTexture.SetTexture( uTex );
	m_pVolumeMaterial->SetVolumeTexture( uTex );
	m_pVolumeMaterial->SetVolumeTextureSize( m_clVolumeTexture.GetTextureSize() );
}

void VolumeMesh::SetVolumeTexure( const String& szBaseTexPath, uint uStartIndex, uint uEndIndex, const String& szExtension )
{
	if( m_clVolumeTexture.SetTexture3D( szBaseTexPath, uStartIndex, uEndIndex, szExtension ) )
	{
		m_pVolumeMaterial->SetVolumeTexture( m_clVolumeTexture.getGlLocation() );
		m_pVolumeMaterial->SetVolumeTextureSize( m_clVolumeTexture.GetTextureSize() );
	}
}


void VolumeMesh::SetTransferFunktionTexture( const String& szPath )
{
	if( m_clTransferfunktionTexture.SetTexture1D( szPath ) )
	{
		m_pVolumeMaterial->SetTransferFunctionTexture( m_clTransferfunktionTexture.getGlLocation() );
		m_pVolumeMaterial->SetTransferFunctionTextureSize( m_clTransferfunktionTexture.GetTextureSize() );
	}	
}

void VolumeMesh::SetTransferFunktionTexture( GLuint uTex )
{
	m_clTransferfunktionTexture.SetTexture( uTex );
	m_pVolumeMaterial->SetTransferFunctionTexture( uTex );
	m_pVolumeMaterial->SetTransferFunctionTextureSize( m_clTransferfunktionTexture.GetTextureSize() );
}

void VolumeMesh::initMeshAndMaterial()
{
	m_pMatRasterize = new MAT_VolCube_Rasterize();
	m_pMatRasterize->Init();

	std::vector<glm::vec3> vPositions;
	vPositions.push_back( glm::vec3( -1.0, -1.0, -1.0 ) ); //0
	vPositions.push_back( glm::vec3( 1.0, -1.0, -1.0 ) );	//1
	vPositions.push_back( glm::vec3( 1.0, 1.0, -1.0 ) );	//2
	vPositions.push_back( glm::vec3( -1.0, 1.0, -1.0 ) );	//3
	vPositions.push_back( glm::vec3( 1.0, -1.0, 1.0 ) );	//4
	vPositions.push_back( glm::vec3( 1.0, 1.0, 1.0 ) );		//5
	vPositions.push_back( glm::vec3( -1.0, 1.0, 1.0 ) );	//6
	vPositions.push_back( glm::vec3( -1.0, -1.0, 1.0 ) );	//7

	glm::uint uIndices[ 24 ] = 
	{
		3, 2, 1, 0, //front
		2, 5, 4, 1, //right
		5, 6, 7, 4, //back
		6, 3, 0, 7, //left
		0, 1, 4, 7, //bottom
		6, 5, 2, 3 //top
	};


	GLuint uVBO;
	glGenBuffers( 1, &uVBO );
	glBindBuffer( GL_ARRAY_BUFFER, uVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * 8, &vPositions[0], GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	GLuint uIBO;
	glGenBuffers( 1, &uIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, uIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint ) * 24, uIndices, GL_STATIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	VertexDeclaration* pVertexDeclaration = new VertexDeclaration();

	pVertexDeclaration->AddVertexElement( VertexElement( 0, Vertex::DataType::FLOAT3, ShaderSemantics::POSITION ) );
	pVertexDeclaration->SetPrimitiveType( GL_QUADS );
	pVertexDeclaration->SetUseIndices( true );
	pVertexDeclaration->SetVertexBufferLoc( uVBO );
	pVertexDeclaration->SetVertexCount( 8 );
	pVertexDeclaration->SetIndexBufferLoc( uIBO );
	pVertexDeclaration->SetIndexCount( 24 );

	m_clVolumeBoundingCube.Create( pVertexDeclaration, m_pMatRasterize, vPositions );
}


