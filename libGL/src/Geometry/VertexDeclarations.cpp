#include "VertexDeclarations.h"

void VertexDeclaration::AddVertexElement( uint16 u16Offset, Vertex::DataType::EDataType dataType, ShaderSemantics::Semantic semantic )
{
	AddVertexElement( VertexElement( u16Offset, dataType, semantic ) );
}

void VertexDeclaration::AddVertexElement(  const VertexElement& element )
{
	m_vElements.push_back( element ),
	m_u16Stride = ComputeCurrentOffset();
}

uint VertexDeclaration::ComputeCurrentOffset()
{
	uint offset = 0;
	for( uint i = 0; i < m_vElements.size(); ++i )
	{
		switch( m_vElements[ i ].GetDataType() )
		{
		case Vertex::DataType::FLOAT1:
			offset += sizeof( float );
			break;

		case Vertex::DataType::FLOAT2:
			offset += sizeof( glm::vec2 );
			break;

		case Vertex::DataType::FLOAT3:
			offset += sizeof( glm::vec3 );
			break;

		case Vertex::DataType::FLOAT4:
			offset += sizeof( glm::vec4 );
			break;
		}
	}

	return offset;
}

bool VertexDeclaration::GetVertexElement( ShaderSemantics::Semantic eSemantic, const VertexElement** ppElement ) const
{  
	for( uint i = 0; i < m_vElements.size(); ++i )
	{
		if( m_vElements[ i ].GetSemantic() == eSemantic )
		{
			*ppElement = &m_vElements[ i ];
			return true;
		}
	}
	return false;
}