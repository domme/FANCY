#include "Model.h"

Model::Model()
{

}

Model::Model( Model& other )
{
	this->m_clAABB = other.m_clAABB;
	this->m_clBoundingSphere = other.m_clBoundingSphere;
	this->m_szName = other.m_szName;

	m_vMeshes.clear();
	for( int i = 0; i < other.m_vMeshes.size(); ++i )
	{
		m_vMeshes.push_back( std::unique_ptr<Mesh>( new Mesh( *other.m_vMeshes[ i ] ) ) );
	}
}

Model::Model( Model&& other )
{
	*this = std::move( other );
}

Model& Model::operator= ( Model&& other )
{
	if( this == &other )
		return *this;

	m_vMeshes = std::move( other.m_vMeshes );
}

Model::~Model()
{

}


void Model::AddMesh( std::unique_ptr<Mesh> pNewMesh )
{
	m_clAABB.combine( pNewMesh->GetAABB() );
	m_clBoundingSphere.combine( pNewMesh->GetBoundingSphere() );

	m_vMeshes.push_back( std::move( pNewMesh ) );
}