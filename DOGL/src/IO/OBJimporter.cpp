#include "../includes.h"

#include "OBJimporter.h"

#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "FileReader.h"
#include "../Geometry/Mesh.h"
//#include "../Rendering/Materials/MAT_TexturedNormalSpecular.h"

using namespace std;


OBJimporter::OBJimporter() : m_pBufferUploader( NULL ), m_szBaseFolder( "" ), m_fUVScaleFact( 1 )
{
	m_pBufferUploader = &GLBufferUploader::GetInstance();
}

OBJimporter::~OBJimporter()
{
}

OBJimporter& OBJimporter::getInstance()
{
	static OBJimporter rInstance;
	
	return rInstance;
}

bool OBJimporter::loadMesh( float fUVscaling )
{
	bool bFirstRun = true;
	bool bMeshEnd = false;
		
	while( !m_stream.eof() )
	{		
		if( bMeshEnd )
		{
			if( m_stream.peek() == 'v' || m_stream.peek() == 'u' )
			{
				return false;
			}
		}

		//walk through line...
		uint uLinePos = 1;
		m_c = m_stream.get();
		while( m_c != '\n' && !m_stream.eof() )
		{
			if( uLinePos == 1 )
			{
				switch ( m_c )
				{
					case 'v':
						paseVertexTypes();
						break;
					case 'f':
						bMeshEnd = true;
						parseFaces();
						break;
					case 'm':
						importMtlLib();
						break;
					case 'g':
						parseMeshName();
						break;
					case 'u':
						parseMeshUseMaterial();
						break;
					default:
						break;
				}
			}

			uLinePos++;
			m_c = m_stream.get();
		}
	}

	m_stream.close();
	return true;

}

void OBJimporter::parseMeshUseMaterial()
{
	do
	{
		m_c = m_stream.get();
	} while( m_c != ' ' ); //jump to end of "usemtl"

	char cBuf[40];
	int idx = 0;
	memset( cBuf, 0, 40 );

	do
	{
		cBuf[ idx++ ] = m_stream.get();
	} while( m_stream.peek() != '\n' );

	//cbuf now contains the name of the material which keys into m_mapMtlDescs
	

	std::map<String, SMtlDesc>::iterator iter = m_mapMaterialDescs.find( cBuf );
	if( iter != m_mapMaterialDescs.end() )
	{
		if( iter->second.szDiffTexName != "" )
		{
			m_pCurrMtlDesc = &iter->second;
		}
	}
}

void OBJimporter::parseMeshName()
{
	m_stream.get();

	char cBuf[40];
	int idx = 0;
	memset( cBuf, 0, 40 );

	do
	{
		cBuf[ idx++ ] = m_stream.get();
	} while( m_stream.peek() != '\n' );

	m_pCurrMesh->m_szName = cBuf;	
}

void OBJimporter::importMtlLib()
{
	char cBuf[40];
	int idx = 0;
	memset( cBuf, 0, 40 );

	do
	{
		m_c = m_stream.get();
	} while( m_c != ' ' );

	//now the next character read is the beginning of the mtllib filename

	std::stringstream sStream;

	do 
	{
		sStream << (char) m_stream.get();
	} while ( m_stream.peek() != '\n' );

	//cBuf[ idx ] = '\0';

	//cBuf now contains the name of the mtlLib to load

	String szMtlPath = m_szBaseFolder + sStream.str();
	FileReader::OpenFileStream( szMtlPath, m_mtlStream );

	assert( m_mtlStream.good() );

	while( !m_mtlStream.eof() )
	{
		uint uLinePos = 1;
		m_mtlC = m_mtlStream.get();
		//walk through line...
		while( m_mtlC != '\n' && !m_mtlStream.eof() )
		{
			if( uLinePos == 1 )
			{
				switch( m_mtlC )
				{
					case 'n' : //"newmtl"
					parseMtlName();
					break;

					case 'b' : //"bump"
					case 'm' : //"map_Kd"
					parseMtlTex();
					break;

				}
			}

			uLinePos++;
			m_mtlC = m_mtlStream.get();
		}
	}

	m_mtlStream.close();
}

void OBJimporter::parseMtlName()
{
	while( m_mtlStream.peek() != ' ' )
	{
		m_mtlStream.get();
	}

	m_mtlStream.get();

	SMtlDesc newMtl;
	char cBuf[50];
	int idx = 0;
	memset( cBuf, 0, 50 );

	do 
	{
		cBuf[ idx++ ] = m_mtlStream.get();
	} while ( m_mtlStream.peek() != '\n' );

	String szMtlName = cBuf;
	m_mapMaterialDescs[ szMtlName ] = newMtl;
	m_pCurrMtlDesc = &m_mapMaterialDescs[ szMtlName ];
}

void OBJimporter::parseMtlTex()
{
	char cBuf[50];
	int idx = 0;
	memset( cBuf, 0, 50 );

	do 
	{
		cBuf[ idx++ ] = m_mtlStream.get();
	} while ( m_mtlStream.peek() != ' ' );

	m_mtlStream.get();

	if( cBuf[ idx - 1 ] == 'd' ) //map_Kd
	{
		int idx = 0;
		memset( cBuf, 0, 50 );

		do 
		{
			cBuf[ idx++ ] = m_mtlStream.get();
		} while ( m_mtlStream.peek() != '\n' );

		m_pCurrMtlDesc->szDiffTexName = m_szBaseFolder + cBuf;
	}


	else if( cBuf[ idx - 1 ] == 'p' ) //bump
	{
		int idx = 0;
		memset( cBuf, 0, 50 );

		do 
		{
			cBuf[ idx++ ] = m_mtlStream.get();
		} while ( m_mtlStream.peek() != ' ' );

		m_pCurrMtlDesc->szBumpTexName = m_szBaseFolder + cBuf;

		do 
		{
			m_mtlStream.get();
		} while ( m_mtlStream.peek() != ' ' );

		m_mtlStream.get();

		idx = 0;
		memset( cBuf, 0, 50 );

		do 
		{
			cBuf[ idx++ ] = m_mtlStream.get();
		} while ( m_mtlStream.peek() != '\n' );

		m_pCurrMtlDesc->fBumpIntensity = atof( cBuf );
	}
}

void OBJimporter::paseVertexTypes()
{
	m_c = m_stream.get();

	switch( m_c )
	{
		case 'n':
			m_stream.get();
			parseNormal();
			break;
		case 't':
			m_stream.get();
			parseTextureCoord();
			break;
		case ' ':
			parseVertexCoord();
			break;
		default:
			break;
	}
}

void OBJimporter::parseVertexCoord()
{
	char cBuf[ 15 ];
	int iIdx = 0;
	memset( cBuf, 0, 15 );
	glm::vec3 vec3Buf;
		
	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != ' ' );

	vec3Buf.x = atof( cBuf );
	iIdx = 0;
	memset( cBuf, 0, 15 );
	m_stream.get();

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != ' ' );

	vec3Buf.y = atof( cBuf );
	iIdx = 0;
	memset( cBuf, 0, 15 );
	m_stream.get();

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != '\n' );

	vec3Buf.z = atof( cBuf );

	vertexCoords.push_back( vec3Buf );
}

void OBJimporter::parseNormal()
{
	char cBuf[ 15 ];
	int iIdx = 0;
	memset( cBuf, 0, 15 );
	glm::vec3 vec3Buf;

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != ' ' );

	vec3Buf.x = atof( cBuf );
	iIdx = 0;
	memset( cBuf, 0, 15 );
	m_stream.get();

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != ' ' );

	vec3Buf.y = atof( cBuf );
	iIdx = 0;
	memset( cBuf, 0, 15 );
	m_stream.get();

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != '\n' );

	vec3Buf.z = atof( cBuf );

	normals.push_back( vec3Buf );
}

void OBJimporter::parseTextureCoord()
{
	char cBuf[ 15 ];
	int iIdx = 0;
	memset( cBuf, 0, 15 );
	glm::vec2 vec2Buf;

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != ' ' );

	vec2Buf.x = atof( cBuf ) * m_fUVScaleFact;
	iIdx = 0;
	memset( cBuf, 0, 15 );
	m_stream.get();

	do 
	{
		cBuf[ iIdx++ ] = m_stream.get();
	} while( m_stream.peek() != '\n' );

	vec2Buf.y = atof( cBuf ) * m_fUVScaleFact;

	textureCoords.push_back( vec2Buf );
}

void OBJimporter::parseFaces()
{
	if( !m_pCurrMesh->m_pMaterial )
	{
		if( m_pCurrMtlDesc && m_pCurrMtlDesc->szDiffTexName != "" )
		{
			/*MAT_TexturedNormalSpecular* pMat = new MAT_TexturedNormalSpecular();
			m_pCurrMesh->m_pMaterial = pMat;
			pMat->SetDiffuseTexture( m_pCurrMtlDesc->szDiffTexName );*/
		}

		if( m_pCurrMesh->m_pMaterial && m_pCurrMtlDesc && m_pCurrMtlDesc->szBumpTexName != "" )
		{
			/*MAT_TexturedNormalSpecular* pMat = (MAT_TexturedNormalSpecular*) m_pCurrMesh->m_pMaterial;
			pMat->SetNormalTexture( m_pCurrMtlDesc->szBumpTexName );
			pMat->SetBumpIntensity( m_pCurrMtlDesc->fBumpIntensity + 2 ); */
		}
	}

	//this code assumes triangles!!
	int iFaceCount = 0;
	
	while( m_stream.peek() != '\n' && !m_stream.eof() )
	{
		char cBuf[ 20 ];
		int idx = 0;
		memset( cBuf, 0, 20 );

		do 
		{
			cBuf[ idx++ ] = m_stream.get();
		} while ( m_stream.peek() != '/' );

		indices.push_back( atoi( cBuf ) - 1 ); //make index buffer 0-based

		m_stream.get();
		idx = 0;
		memset( cBuf, 0, 20 );

		do 
		{
			cBuf[ idx++ ] = m_stream.get();
		} while ( m_stream.peek() != '/' );

		textureIndices.push_back( atoi( cBuf ) - 1 );

		m_stream.get();
		idx = 0;
		memset( cBuf, 0, 20 );

		do 
		{
			cBuf[ idx++ ] = m_stream.get();
		} while ( m_stream.peek() != ' ' && m_stream.peek() != '\n' && !m_stream.eof() );

		normalIndices.push_back( atoi( cBuf ) - 1 );


		//build actual vertices and indices
		uint uCurrIndex = indices.size() - 1;

		SIndexTupel sTupel;
		sTupel.uPosIdx = indices[ uCurrIndex ];
		sTupel.uTexIdx = textureIndices[ uCurrIndex ];
		sTupel.uNormIdx = normalIndices[ uCurrIndex ];

		uint uIndex;
		std::map<SIndexTupel, uint>::const_iterator iter;
		iter = m_mapTupelNewIndex.find( sTupel );

		if( iter == m_mapTupelNewIndex.end() ) //no index for this vertex saved! So Create a new vertex!
		{
			Vertex::PosNormTexTan vert;
			vert.Position = vertexCoords[ sTupel.uPosIdx ];
			vert.Normal = normals[ sTupel.uNormIdx ];
			vert.TextureCoord = textureCoords[ sTupel.uTexIdx ];
			
			m_vVertices.push_back( vert );
			uIndex = m_vVertices.size() - 1;

			m_mapTupelNewIndex[ sTupel ] = uIndex;
			m_vIndices.push_back( uIndex );
			m_tan1List.push_back( glm::vec3( 0.0f, 0.0f, 0.0f ) );
			m_tan2List.push_back( glm::vec3( 0.0f, 0.0f, 0.0f ) );
		}

		else
		{
			m_vIndices.push_back( iter->second );
			uIndex = iter->second;
			//since this vertex was used by a face once before, we increment its counter
		}

		//now look up if this position was used by an other face before and increment the counter if neccessary
		std::map<SVertexPos, uint>::iterator counterIter;
		SVertexPos sPos;
		sPos.v3Pos = vertexCoords[ sTupel.uPosIdx ];
		counterIter = m_mapFaceCounters.find( sPos );

		if( counterIter == m_mapFaceCounters.end() )
		{
			m_mapFaceCounters[ sPos ] = 1;
		}

		else
		{
			counterIter->second++;
		}

		iFaceCount++;
	}
	
	buildTangentsForLastFace();
}

void OBJimporter::buildTangentsForLastFace()
{
	//Tangent-space calculation implementation as described in "Math for 3D Game Programming & Computer Graphics (Charles River Media Game Development)" by Eric Lengyel
	
	//get the indices for the last built
	uint i1 = m_vIndices[ m_vIndices.size() - 3 ];
	uint i2 = m_vIndices[ m_vIndices.size() - 2 ];
	uint i3 = m_vIndices[ m_vIndices.size() - 1 ];
	
	Vertex::PosNormTexTan* pv1 = &m_vVertices[ i1 ];
	Vertex::PosNormTexTan* pv2 = &m_vVertices[ i2 ];
	Vertex::PosNormTexTan* pv3 = &m_vVertices[ i3 ];
	
	glm::vec3 v1= pv1->Position;
	glm::vec3 v2= pv2->Position;
	glm::vec3 v3= pv3->Position;
	
	glm::vec2 w1 = pv1->TextureCoord;
	glm::vec2 w2 = pv2->TextureCoord;
	glm::vec2 w3 = pv3->TextureCoord;

	float x1 = v2.x - v1.x;
	float x2 = v3.x - v1.x;
	float y1 = v2.y - v1.y;
	float y2 = v3.y - v1.y;
	float z1 = v2.z - v1.z;
	float z2 = v3.z - v1.z;

	float s1 = w2.x - w1.x;
	float s2 = w3.x - w1.x;
	float t1 = w2.y - w1.y;
	float t2 = w3.y - w1.y;

	float r  = 1.0f / (s1 * t2 - s2 * t1 );
	glm::vec3 v3S( ( t2 * x1 - t1 * x2 ) * r, ( t2 * y1 - t1 * y2 ) * r, ( t2 * z1 - t1 * z2 ) * r );
	glm::vec3 v3T( ( s1 * x2 - s2 * x1 ) * r, ( s1 * y2 - s2 * y1 ) * r, ( s1 * z2 - s2 * z1 ) * r );

	m_tan1List[ i1 ] += v3S;
	m_tan1List[ i2 ] += v3S;
	m_tan1List[ i3 ] += v3S;

	m_tan2List[ i1 ] += v3T;
	m_tan2List[ i2 ] += v3T;
	m_tan2List[ i3 ] += v3T;
}

void OBJimporter::postProcessVertices()
{
	for( int i = 0; i < m_vVertices.size(); ++i )
	{
		SVertexPos sPos;
		sPos.v3Pos = m_vVertices[i].Position;
		uint uCounter = m_mapFaceCounters[ sPos ];
		glm::vec3 v3Tangent = m_tan1List[ i ] / (float) uCounter; //average the tangent
		glm::vec3& v3Normal = m_vVertices[i].Normal;
		
		m_vVertices[i].Tangent = v3Tangent; 
	}
}

unique_ptr<Mesh> OBJimporter::loadMeshFromFile( const std::string& szFilename, float fUVscaling /*= 1.0f*/ )
{
	clearTemporalData();

	//getBaseFolder( szFilename );

	m_fUVScaleFact = fUVscaling;
	FileReader::OpenFileStream( szFilename, m_stream );

	m_pCurrMesh = std::unique_ptr<Mesh>( new Mesh() );
	
	loadMesh( fUVscaling );
	postProcessVertices();

  	GLuint uVBO = m_pBufferUploader->UploadBufferData( &m_vVertices[ 0 ], m_vVertices.size(), sizeof( Vertex::PosNormTexTan ), GL_ARRAY_BUFFER, GL_STATIC_DRAW );
	GLuint uIBO = m_pBufferUploader->UploadBufferData( &m_vIndices[ 0 ], m_vIndices.size(), sizeof( uint32 ), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW );

	VertexDeclaration* pVertexInfo = new VertexDeclaration();
	pVertexInfo->AddVertexElement( 0,												Vertex::DataType::FLOAT3, ShaderSemantics::POSITION ); //Pos
	pVertexInfo->AddVertexElement( sizeof( glm::vec3 ),								Vertex::DataType::FLOAT3, ShaderSemantics::NORMAL ); //Norm
	pVertexInfo->AddVertexElement( 2 * sizeof( glm::vec3 ),							Vertex::DataType::FLOAT2, ShaderSemantics::UV0 ); //Tex
	pVertexInfo->AddVertexElement( 2 * sizeof( glm::vec3 ) + sizeof( glm::vec2 ),	Vertex::DataType::FLOAT3, ShaderSemantics::TANGENT ); //Tan
	//pVertexInfo->SetStride( sizeof( Vertex::PosNormTexTan ) );
	pVertexInfo->SetIndexCount( m_vIndices.size() );
	pVertexInfo->SetUseIndices( true );
	pVertexInfo->SetVertexCount( m_vVertices.size() );
	pVertexInfo->SetVertexBufferLoc( uVBO );
	pVertexInfo->SetIndexBufferLoc( uIBO );
	pVertexInfo->SetPrimitiveType( GL_TRIANGLES );
	m_pCurrMesh->m_pVertexInfo = pVertexInfo;
		
	m_pCurrMesh->Init( vertexCoords );
	
	clearTemporalData();

	return std::move( m_pCurrMesh );
}

//String OBJimporter::getBaseFolder( const String& szFilename )
//{
//	size_t iPos = szFilename.find_last_of( "/" );
//
//	if( iPos != String::npos )
//	{
//		m_szBaseFolder = szFilename.substr( 0, iPos + 1 );
//	}
//	
//	return m_szBaseFolder;
//}

void OBJimporter::clearTemporalData()
{
	vertexCoords.clear();
	textureCoords.clear();
	normals.clear();
	indices.clear();
	normalIndices.clear();
	textureIndices.clear();
	m_mapTupelNewIndex.clear();
	m_pCurrMtlDesc = NULL;
	m_szBaseFolder  = "";
	m_mtlStream.close();
	m_stream.close();
	m_mapFaceCounters.clear();
	m_tan1List.clear();
	m_tan2List.clear();
	m_fUVScaleFact = 1;
	m_vVertices.clear();
	m_vIndices.clear();
}
