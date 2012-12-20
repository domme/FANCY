#ifndef VERTEXDECLARATIONS_H
#define VERTEXDECLARATIONS_H

#include "../Includes.h"

#include "../Rendering/ShaderSemantics.h"


namespace Vertex
{
	struct DLLEXPORT VertexType
	{
		glm::vec3 Position;
	};

	struct DLLEXPORT PosTex : VertexType
	{
		glm::vec2 TextureCoord;
	};

	struct DLLEXPORT PosNormTex : VertexType
	{
		glm::vec3 Normal;
		glm::vec2 TextureCoord;
	};
	
	struct DLLEXPORT PosNormTexTan : PosNormTex
	{
		glm::vec3 Tangent;
	};

	namespace DataType
	{
		enum EDataType
		{
			FLOAT1 = 1,
			FLOAT2,
			FLOAT3,
			FLOAT4,

			NUM
		};
	}
}

class DLLEXPORT VertexElement
{
public:
	VertexElement() : m_u16Offset( 0 ), m_eDataType( Vertex::DataType::FLOAT3 ), m_eShaderSemantic( ShaderSemantics::POSITION ), m_u8DataCount( 1 ) {}
	VertexElement( uint16 offset, Vertex::DataType::EDataType dataType, ShaderSemantics::Semantic semantic ) { m_u16Offset = offset; m_eDataType = dataType; m_eShaderSemantic = semantic; m_u8DataCount = static_cast<uint8>( dataType ); }
	~VertexElement() {}

	uint16						GetOffset() const { return m_u16Offset; }
	Vertex::DataType::EDataType GetDataType() const { return m_eDataType; }
	uint8						GetDataCount() const { return m_u8DataCount; }
	ShaderSemantics::Semantic	GetSemantic() const { return m_eShaderSemantic; }

protected:
	Vertex::DataType::EDataType		m_eDataType;
	ShaderSemantics::Semantic		m_eShaderSemantic;
	uint16							m_u16Offset;
	uint8							m_u8DataCount;
	uint8							_padding;
};

class DLLEXPORT VertexDeclaration 
{

friend class ModelLoader;

public:
	VertexDeclaration() : m_u16Stride( 0 ), m_u32IndexCount( 0 ), m_u32VertexCount( 0 ), m_bUseIndices( true ) {}
	VertexDeclaration( VertexDeclaration& other ) : m_u16Stride( 0 ), m_u32IndexCount( 0 ), m_u32VertexCount( 0 ), m_bUseIndices( true )
	{ 
		//Only primitive data types - regular "=" will suffice
		//Resource Manager will be notified in the Mesh-Setter
		*this = other;
	}

	~VertexDeclaration() {}

	uint16								GetStride() const { return m_u16Stride; }
	const std::vector<VertexElement>&	GetVertexElements() const { return m_vElements; }
	uint32								GetVertexCount() const { return m_u32VertexCount; }
	uint32								GetIndexCount() const { return m_u32IndexCount; }
	bool								GetUseIndices() const { return m_bUseIndices; }
	GLuint								GetVertexBufferLoc() const { return m_uVertexBufferLoc; }
	GLuint								GetIndexBufferLoc() const { return m_uIndexBufferLoc; }
	GLuint								GetPrimitiveType() const { return m_uPrimitiveType; }
	bool								GetVertexElement( ShaderSemantics::Semantic eSemantic, const VertexElement** ppElement ) const;

	void AddVertexElement( uint16 u16Offset, Vertex::DataType::EDataType dataType, ShaderSemantics::Semantic semantic );
	void AddVertexElement( const VertexElement& element );

	//void SetStride( uint16 u16Stride ) { m_u16Stride = u16Stride; }
	void SetUseIndices( bool bIndices ) { m_bUseIndices = bIndices; }
	void SetVertexCount( uint32 u32VertexCount ) { m_u32VertexCount = u32VertexCount; }
	void SetIndexCount( uint32 u32IndexCount ) { m_u32IndexCount = u32IndexCount; }
	void SetVertexBufferLoc( GLuint loc ) { m_uVertexBufferLoc = loc; }
	void SetIndexBufferLoc( GLuint loc ) { m_uIndexBufferLoc = loc; }
	void SetPrimitiveType( GLuint uType ) { m_uPrimitiveType = uType; }
	uint ComputeCurrentOffset();

protected:
	uint32 m_u32VertexCount;
	uint32 m_u32IndexCount;

	GLuint	m_uVertexBufferLoc;
	GLuint	m_uIndexBufferLoc;
	GLuint	m_uPrimitiveType;

	bool m_bUseIndices;
	uint16 m_u16Stride;
	std::vector<VertexElement> m_vElements;
	
};

#endif