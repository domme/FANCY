#include "ModelLoader.h"
#include "../Geometry/Mesh.h"
#include "../Geometry/VertexDeclarations.h"

#include "PathService.h"

#include "assimp/Logger.h"
#include "assimp/DefaultLogger.h"

//Materials
#include "../Rendering/Managers/GLBufferUploader.h"
#include "../Rendering/Managers/GLResourcePathManager.h"

#include "PathService.h"




ModelLoader::ModelLoader()
{
	const uint uLogSeverity = Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Err | Assimp::Logger::Warn;
	Assimp::DefaultLogger::get()->attachStream( &m_clLogStream, uLogSeverity );
}

ModelLoader::~ModelLoader()
{

}

Mesh* ModelLoader::LoadSingleMeshGeometry(  const String& szModelPath )
{
	String szAbsPath = PathService::convertToAbsPath( szModelPath );
	String szModelFolder = PathService::GetContainingFolder( szModelPath );

	const aiScene* pAiScene = m_aiImporter.ReadFile( szAbsPath, aiProcess_CalcTangentSpace |
																aiProcess_JoinIdenticalVertices |
																aiProcess_Triangulate );

	if( !pAiScene )
	{
		LOG( String( "Import failed of Scene : " ) + szModelPath );
		return NULL;
	}

	if( pAiScene->mNumMeshes == 0 )
	{
		LOG( String( "No Mesh in Scene " ) + szModelPath );
		return NULL;
	}

	else if( pAiScene->mNumMeshes > 1 )
		LOG( String( "WARNING: Loading single Mesh from File with more Meshes! " ) + szModelPath );


	return ProcessMesh( pAiScene, pAiScene->mMeshes[ 0 ], szModelPath, NULL, 0, false );
}



Mesh* ModelLoader::ProcessMesh( const aiScene* pAiScene, aiMesh* paiMesh, const String& szModelPath, Material** vpMaterials, uint i, bool assignMaterial /* = true */ )
{
	GLVBOpathManager& rVBOpathMgr = GLVBOpathManager::GetInstance();
	GLIBOpathManager& rIBOpathMgr = GLIBOpathManager::GetInstance();
	GLBufferUploader& rBufferUploader = GLBufferUploader::GetInstance();

	Mesh* pMesh = new Mesh;
	
	String szName = "";

	if( paiMesh->mName.length > 0 )
		szName = szModelPath + std::string( paiMesh->mName.data );

	else
	{
		std::stringstream ss;
		ss << szModelPath << "_Mesh_" << i;
		szName = ss.str();
	}

	pMesh->SetName( szName );
		
	if( assignMaterial )
		pMesh->SetMaterial( vpMaterials[ paiMesh->mMaterialIndex ]->Clone() );

	VertexDeclaration* pVertexInfo = new VertexDeclaration;
	
	if( !paiMesh->HasPositions() )
	{
		LOG( std::string( "ERROR: Mesh " ) + pMesh->GetName() + std::string( " has no Vertex Positions!" ) );
		delete pMesh;
		return NULL;
	}
	
	//POSITION
	pVertexInfo->AddVertexElement( VertexElement( 0, Vertex::DataType::FLOAT3, ShaderSemantics::POSITION ) );
	
	//NORMAL
	if( paiMesh->HasNormals() )
		pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT3, ShaderSemantics::NORMAL );

	//UVs
	for( unsigned int i = 0; i < paiMesh->GetNumUVChannels() && i < ShaderSemantics::MAX_UV_CHANNELS; ++i )
		if( paiMesh->HasTextureCoords( i ) )
			pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT2, (ShaderSemantics::Semantic) ( ShaderSemantics::UV0 + i ) );
	
	//TANGENT
	if( paiMesh->HasTangentsAndBitangents() )
		//Currently only support for Tangents. Bitangents are calculated in the shaders
		pVertexInfo->AddVertexElement( pVertexInfo->ComputeCurrentOffset(), Vertex::DataType::FLOAT3, ShaderSemantics::TANGENT );

	
	//TODO: Support bones, VertexColors etc. in the future!


	if( !rVBOpathMgr.HasResource( pMesh->GetName() ) )
	{
		uint8* pVBOdata = (uint8*) malloc( pVertexInfo->GetStride() * paiMesh->mNumVertices );
		memset( pVBOdata, 0, pVertexInfo->GetStride() * paiMesh->mNumVertices );

		const std::vector<VertexElement>& vVertexElements = pVertexInfo->GetVertexElements();


		for( unsigned int i = 0; i < paiMesh->mNumVertices; ++i )
		{
			uint8* pVertex = &pVBOdata[ pVertexInfo->GetStride() * i ];

			for( unsigned int iVelement = 0; iVelement < vVertexElements.size(); ++iVelement )
			{
				const VertexElement& clElement = vVertexElements[ iVelement ];

				switch( clElement.GetSemantic() )
				{
				case ShaderSemantics::POSITION:
					{
						aiVector3D pos = paiMesh->mVertices[ i ];
						glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
						pVal->x = pos.x;
						pVal->y = pos.y;
						pVal->z = pos.z;

						++pVal;
						pVertex = (uint8*) pVal;
					}
					break;

				case ShaderSemantics::NORMAL:
					{
						aiVector3D norm = paiMesh->mNormals[ i ];
						glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
						pVal->x = norm.x;
						pVal->y = norm.y;
						pVal->z = norm.z;

						++pVal;
						pVertex = (uint8*) pVal;

					}
					break;

				case ShaderSemantics::UV0:
				case ShaderSemantics::UV1:
				case ShaderSemantics::UV2:
				case ShaderSemantics::UV3:
				case ShaderSemantics::UV4:
				case ShaderSemantics::UV5:
				case ShaderSemantics::UV6:
				case ShaderSemantics::UV7:
					{
						aiVector3D uv = (paiMesh->mTextureCoords[ clElement.GetSemantic() - ShaderSemantics::UV0 ])[ i ];
						glm::vec2* pVal = reinterpret_cast<glm::vec2*>( pVertex );
						pVal->x = uv.x;
						pVal->y = uv.y;

						++pVal;
						pVertex = (uint8*) pVal;
					}
					break;

				case ShaderSemantics::TANGENT:
					{
						aiVector3D tangent = paiMesh->mTangents[ i ];
						glm::vec3* pVal = reinterpret_cast<glm::vec3*>( pVertex );
						pVal->x = tangent.x;
						pVal->y = tangent.y;
						pVal->z = tangent.z;

						++pVal;
						pVertex = (uint8*) pVal;
					}
					break;
				}
			}
		}

		GLuint uVBO = rBufferUploader.UploadBufferData( pVBOdata, paiMesh->mNumVertices, pVertexInfo->GetStride(), GL_ARRAY_BUFFER, GL_STATIC_DRAW );

		rVBOpathMgr.AddResource( pMesh->GetName(), uVBO );

		free( pVBOdata );

	} //end if( !rVBOpathMgr.HasResource( pMesh->GetName() ) )

	pVertexInfo->m_uVertexBufferLoc = rVBOpathMgr.GetResource( pMesh->GetName() );
	pVertexInfo->m_u32VertexCount = paiMesh->mNumVertices;
	pVertexInfo->m_bUseIndices = paiMesh->HasFaces();
	pVertexInfo->m_uPrimitiveType = GL_TRIANGLES;

	if( paiMesh->HasFaces() )
	{
		if( !rIBOpathMgr.HasResource( pMesh->GetName() ) )
		{
			//NOTE: Assuming that faces are always triangles
			uint uNumIndices = paiMesh->mNumFaces * 3;

			uint* pIBOdata = (uint*) malloc( sizeof( uint ) * uNumIndices );

			uint uIdx = 0;
			for( unsigned int iFace = 0; iFace < paiMesh->mNumFaces; ++iFace )
			{
				aiFace& rFace = paiMesh->mFaces[ iFace ];

				for( unsigned int iFaceIndex = 0; iFaceIndex < rFace.mNumIndices; ++iFaceIndex )
				{
					pIBOdata[ uIdx++ ] = rFace.mIndices[ iFaceIndex ];
				}
			}
			
			GLuint uIBO = rBufferUploader.UploadBufferData( pIBOdata, uNumIndices, sizeof( uint ), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW );

			rIBOpathMgr.AddResource( pMesh->GetName(), uIBO );

			free( pIBOdata );
		}

		pVertexInfo->m_u32IndexCount = paiMesh->mNumFaces * 3;
		pVertexInfo->m_uIndexBufferLoc = rIBOpathMgr.GetResource( pMesh->GetName() );

	} //end if( paiMesh->HasFaces() )
		
	
	//Translate ai-positions to glm-positions
	//Ugly but necessary for mesh-initialization for now
	std::vector<glm::vec3> vPositions;
	for( unsigned int i = 0; i < paiMesh->mNumVertices; ++i )
		vPositions.push_back( glm::vec3( paiMesh->mVertices[ i ].x, paiMesh->mVertices[ i ].y, paiMesh->mVertices[ i ].z ) );

	pMesh->Init( vPositions );

	//Append the vertex-Info to the mesh. Thereby, the Resource manager gets notified and adds a reference count for the gl-buffers
	pMesh->SetVertexInfo( pVertexInfo );
	
	return pMesh;	
}

glm::mat4 ModelLoader::MatFromAiMat( const aiMatrix4x4& mat )
{
	return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
						mat.b1, mat.b2, mat.b3, mat.b4,
						mat.c1, mat.c2, mat.c3, mat.c4,
						mat.d1, mat.d2, mat.d3, mat.d4 );
}


