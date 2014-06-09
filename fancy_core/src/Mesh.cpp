#include "../includes.h"

#include <assert.h>
#include <iostream>
#include <algorithm>

#include "Mesh.h"
#include "../Rendering/GLRenderer.h"
#include "../Rendering/Materials/Material.h"
#include "../Scene/Camera.h"

#include "../Rendering/Managers/GLResourceManager.h"

Mesh::Mesh( Mesh& other )  :
m_pMaterial( NULL ),
m_pVertexInfo( NULL )
{
	if( other.m_pMaterial )
		m_pMaterial = other.m_pMaterial->Clone();

	if( other.m_pVertexInfo )
		//Use the Setter here to inform the Resource Manager!
		SetVertexInfo( new VertexDeclaration( *other.m_pVertexInfo ) );

	m_clAABB = other.m_clAABB;
	m_clBoundingSphere = other.m_clBoundingSphere;
	m_szName = other.m_szName;
}

Mesh::Mesh( Mesh&& other ) 
{
	*this = std::move( other );
}

Mesh& Mesh::operator= ( Mesh&& other )
{
	if( this == &other )
		return *this;

	SAFE_DELETE( m_pMaterial );
	SAFE_DELETE( m_pVertexInfo );

	m_pMaterial = other.m_pMaterial;

	//Direct assignment of vertex info intended! Resource Manager should not be notified if the Mesh is moved!
	m_pVertexInfo = other.m_pVertexInfo;
	
	m_clAABB = other.m_clAABB;
	m_clBoundingSphere = other.m_clBoundingSphere;
	m_szName = std::move( other.m_szName );

	other.m_pMaterial = NULL;
	other.m_pVertexInfo = NULL;

	return *this;
}

Mesh::~Mesh()
{
	if( m_pVertexInfo )
	{
		VBOResourceManager::GetInstance().HandleDelete( m_pVertexInfo->GetVertexBufferLoc() );

		if( m_pVertexInfo->GetUseIndices() )
			IBOResourceManager::GetInstance().HandleDelete( m_pVertexInfo->GetIndexBufferLoc() );
	}


	SAFE_DELETE( m_pMaterial );
	SAFE_DELETE( m_pVertexInfo );
}

void Mesh::CloneInto( Mesh& other )
{

}

void Mesh::Create( VertexDeclaration* pVertexInfo, Material* pMaterial, const std::vector<glm::vec3>& vertexPositions )
{
	SetVertexInfo( pVertexInfo );
	m_pMaterial = pMaterial;

	Init( vertexPositions );
}

void Mesh::SetVertexInfo( VertexDeclaration* pNewVertexInfo )
{
	VBOResourceManager::GetInstance().AddResource( pNewVertexInfo->GetVertexBufferLoc() );
	
	if( pNewVertexInfo->GetUseIndices() )
		IBOResourceManager::GetInstance().AddResource( pNewVertexInfo->GetIndexBufferLoc() );

	if( m_pVertexInfo )
	{
		VBOResourceManager::GetInstance().HandleDelete( m_pVertexInfo->GetVertexBufferLoc() );

		if( m_pVertexInfo->GetUseIndices() )
			IBOResourceManager::GetInstance().HandleDelete( m_pVertexInfo->GetIndexBufferLoc() );
	}

	m_pVertexInfo = pNewVertexInfo;
}

void Mesh::Init( const std::vector<glm::vec3>& vertexPositions )
{
	initBoundingGeometry( vertexPositions );
}