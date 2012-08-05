#include "FullscreenQuad.h"
#include "VertexDeclarations.h"

#include "Mesh.h"
#include "../Engine.h"
#include "../Rendering/GLRenderer.h"

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

}

void FullscreenQuad::init()
{
	m_pEngine = &Engine::GetInstance();

	m_pMesh = unique_ptr<Mesh>( new Mesh );
	m_pMaterial = new MAT_FSquad_Textured;
	m_pMaterial->Init();

	m_pMatTex3D = new MAT_FSquad_Textured3D;
	m_pMatTex3D->Init();

	
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

void FullscreenQuad::RenderTexture( GLuint uTexture )
{
	static glm::mat4 matIdentity;
	
	m_pMaterial->SetTexture( uTexture );
	m_pEngine->GetRenderer()->RenderMesh( m_pMesh._Myptr, m_pMesh->GetMaterial()->GetForwardShader(), m_pEngine->GetCurrentCamera(), m_pEngine->GetScene(), matIdentity );
}

void FullscreenQuad::RenderTimeTexture3D( GLuint uTexture )
{
	static glm::mat4 matIdentity;

	m_pMatTex3D->SetVolumeTexture( uTexture );

	m_pMesh->SetMaterial( m_pMatTex3D );
	m_pEngine->GetRenderer()->RenderMesh( m_pMesh._Myptr, m_pMatTex3D->GetForwardShader(), m_pEngine->GetCurrentCamera(), m_pEngine->GetScene(), matIdentity );
	m_pMesh->SetMaterial( m_pMaterial );
}

void FullscreenQuad::RenderWithMaterial(  Material* pMat  )
{
	static glm::mat4 matIdentity;

	m_pMesh->SetMaterial( pMat );
	m_pEngine->GetRenderer()->RenderMesh( m_pMesh._Myptr, pMat->GetForwardShader(), m_pEngine->GetCurrentCamera(), m_pEngine->GetScene(), matIdentity );
	m_pMesh->SetMaterial( m_pMaterial );
}

