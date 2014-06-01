#include <Geometry/VertexDeclarations.h>
#include <Geometry/Mesh.h>
#include <Rendering/GLRenderer.h>

#include "FullscreenQuad.h"
#include "../Engine.h"

FullscreenQuad& FullscreenQuad::getInstance()
{
	static FullscreenQuad instance;
	return instance;
}

FullscreenQuad::FullscreenQuad() : m_pMaterial( NULL ), m_pEngine( NULL )
{
	init();
}

FullscreenQuad::~FullscreenQuad()
{
	SAFE_DELETE( m_pMesh );
	//SAFE_DELETE( m_pMaterial );
}

void FullscreenQuad::init()
{
	m_pEngine = &Engine::GetInstance();

	m_pMesh = new Mesh;
	m_pMaterial = new MAT_FSquad_Textured;
	m_pMaterial->Init();


	Vertex::PosTex vertices[ 4 ];

	vertices[ 0 ].Position = glm::vec3( -1.0f, -1.0f, 0.0f );
	vertices[ 0 ].TextureCoord = glm::vec2( 0.0f, 0.0f );

	vertices[ 1 ].Position = glm::vec3( 1.0f, -1.0f, 0.0f );
	vertices[ 1 ].TextureCoord = glm::vec2( 1.0f, 0.0f );

	vertices[ 2 ].Position = glm::vec3( 1.0f, 1.0f, 0.0f );
	vertices[ 2 ].TextureCoord = glm::vec2( 1.0f, 1.0f );

	vertices[ 3 ].Position = glm::vec3( -1.0f, 1.0f, 0.0f );
	vertices[ 3 ].TextureCoord = glm::vec2( 0.0f, 1.0f );

	GLuint uVBO;
	glGenBuffers( 1, &uVBO );
	glBindBuffer( GL_ARRAY_BUFFER, uVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex::PosTex ) * 4, vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	std::vector<glm::vec3> vertexPos;
	vertexPos.push_back( vertices[ 0 ].Position );
	vertexPos.push_back( vertices[ 1 ].Position );
	vertexPos.push_back( vertices[ 2 ].Position );
	vertexPos.push_back( vertices[ 3 ].Position );

	VertexDeclaration* pVertexInfo = new VertexDeclaration;

	pVertexInfo->AddVertexElement( VertexElement( 0, Vertex::DataType::FLOAT3, ShaderSemantics::POSITION ) );
	pVertexInfo->AddVertexElement( VertexElement( sizeof( glm::vec3 ), Vertex::DataType::FLOAT2, ShaderSemantics::UV0 ) );
	pVertexInfo->SetUseIndices( false );
	//pVertexInfo->SetStride( sizeof( Vertex::PosTex ) );
	pVertexInfo->SetVertexCount( 4 );
	pVertexInfo->SetVertexBufferLoc( uVBO );
	pVertexInfo->SetPrimitiveType( GL_QUADS );
	
	m_pMesh->Create( pVertexInfo, m_pMaterial, vertexPos );
}

void FullscreenQuad::RenderTexture( GLuint uTexture, Camera* pCamera )
{
	m_pMaterial->SetTexture( uTexture );
	m_pEngine->GetRenderer()->RenderMesh( m_pMesh, glm::mat4( 1.0f ), pCamera );
}


void FullscreenQuad::RenderWithMaterial( Material* pMat, Camera* pCamera )
{
	m_pMesh->SetMaterial( pMat );
	m_pEngine->GetRenderer()->RenderMesh( m_pMesh, glm::mat4( 1.0f ), pCamera, pMat, pMat->GetShader() );
	m_pMesh->SetMaterial( m_pMaterial );
}

