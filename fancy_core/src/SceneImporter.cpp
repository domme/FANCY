#include "SceneImporter.h"

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialPass.h"
#include "PathService.h"
#include "ModelComponent.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "GeometryVertexLayout.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal
  {
  //---------------------------------------------------------------------------//
    class FancyLog : public Assimp::LogStream
    {
      void write(const char* message) override;
    };
    //---------------------------------------------------------------------------//
    void FancyLog::write(const char* message)
    {
      log_Info(message);
    }
//---------------------------------------------------------------------------//
    glm::mat4 matFromAiMat( const aiMatrix4x4& mat )
    {
      return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
        mat.b1, mat.b2, mat.b3, mat.b4,
        mat.c1, mat.c2, mat.c3, mat.c4,
        mat.d1, mat.d2, mat.d3, mat.d4 );
    }
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    static FancyLog* m_pLogger = nullptr;
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  namespace Processing
  {
    typedef std::map<const aiMesh*, Geometry::Mesh*> MeshCacheMap;

    struct WorkingData
    {
      const aiScene* pCurrScene;
      Fancy::Scene::SceneNode* pFancyParentNode;
      MeshCacheMap mapAiMeshToMesh;
    };

    WorkingData currWorkingData;

    bool processAiScene(const aiScene* _pAscene);
    bool processAiNode(const aiNode* _pAnode, Scene::SceneNode* _pParentNode);
    bool processMeshes(const uint32* _puMeshIndices, const uint32 _uNumMeshes, 
      Scene::ModelComponent* _pModelComponent);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void SceneImporter::initLogger()
  {
    const unsigned int severity = 
        Assimp::Logger::Debugging
        |Assimp::Logger::Info
        |Assimp::Logger::Err
        |Assimp::Logger::Warn;

    Internal::m_pLogger = new Internal::FancyLog();

    Assimp::DefaultLogger::get()->attachStream(Internal::m_pLogger, severity);
  }
//---------------------------------------------------------------------------//
  void SceneImporter::destroyLogger()
  {
    Assimp::DefaultLogger::get()->detatchStream(Internal::m_pLogger);
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::importToSceneGraph( const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode )
  {
    bool success = false;
    std::string szImportPathAbs = PathService::convertToAbsPath(_szImportPathRel);

    Assimp::Importer aImporter;
    
    const aiScene* aScene = aImporter.ReadFile(szImportPathAbs,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (!aScene)
    {
      return false;
    }

    Processing::WorkingData workingData = {0};
    Processing::currWorkingData = workingData;
    Processing::currWorkingData.pFancyParentNode = _pParentNode;
    return Processing::processAiScene(aScene);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  bool Processing::processAiScene(const aiScene* _pAscene)
  {
    const aiNode* pArootNode = _pAscene->mRootNode;

    if (!pArootNode)
    {
      return false;
    }

    currWorkingData.pCurrScene = _pAscene;
    return processAiNode(pArootNode);
  }
//---------------------------------------------------------------------------//
  bool Processing::processAiNode(const aiNode* _pAnode, Scene::SceneNode* _pParentNode)
  {
    bool success = true;

    Scene::SceneNodePtr pNode = std::make_shared<Scene::SceneNode>();
    if (_pAnode->mName.length > 0u)
    {
      pNode->setName(ObjectName(_pAnode->mName.C_Str()));
    }
    
    Scene::SceneNode::
      parentNodeToNode(pNode, _pParentNode);

    pNode->getTransform().setLocal(Internal::matFromAiMat(_pAnode->mTransformation));

    if (_pAnode->mNumMeshes > 0u)
    {
      Scene::ModelComponent* pModelComponent =
        static_cast<Scene::ModelComponent*>(pNode->createComponent(_N(ModelComponent)));

      Processing::processMeshes(_pAnode->mMeshes, _pAnode->mNumMeshes, pModelComponent);
    }
    
    for (uint32 i = 0u; i < _pAnode->mNumChildren; ++i)
    {
      success &= Processing::processAiNode(_pAnode->mChildren[i], pNode.get());
    }

    return success;
  }
//---------------------------------------------------------------------------//
  bool Processing::processMeshes(const uint32* _puMeshIndices, 
    const uint32 _uNumMeshes, Scene::ModelComponent* _pModelComponent)
  {
    // Note: As a first approach, each mesh becomes a submesh of the model
    Geometry::Model* pModel = new Geometry::Model();
    _pModelComponent->setModel(pModel);

    for (uint32 i = 0u; i < _uNumMeshes; ++i)
    {
      const aiMesh* pAmesh = Processing::currWorkingData.pCurrScene->mMeshes[i];
      Processing::MeshCacheMap::iterator it = 
        Processing::currWorkingData.mapAiMeshToMesh.find(pAmesh);

      Geometry::Mesh* pMesh = nullptr;
      if (it != Processing::currWorkingData.mapAiMeshToMesh.end() )
      {
        pMesh = it->second;
      }
      else 
      {
        pMesh = new Geometry::Mesh();
        Processing::currWorkingData.mapAiMeshToMesh[pAmesh] = pMesh;

        if (pAmesh->mName.length > 0u)
        {
          pMesh->setName(ObjectName(pAmesh->mName.C_Str()));
        }

        Geometry::GeometryData* pGeoData = new Geometry::GeometryData();
        

      }

    }
    
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO


// Legacy:
// Modelloder:

/*
const float fGlobalNormalMod = 1.0f;


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


	return processMesh( pAiScene, pAiScene->mMeshes[ 0 ], szModelPath, NULL, 0, false );
}


SceneNode* ModelLoader::LoadAsset( const String& szModelPath, SceneManager* pScene )
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

	Material** vpMaterials = new Material*[ pAiScene->mNumMaterials ];
	for( int i = 0; i < pAiScene->mNumMaterials; ++i )
	{
		vpMaterials[ i ] = processMaterial( pAiScene, pAiScene->mMaterials[ i ], szModelFolder );
	}


	Mesh** vpMeshes = new Mesh*[ pAiScene->mNumMeshes ];
	for( int i = 0; i < pAiScene->mNumMeshes; ++i )
	{
		vpMeshes[ i ] = processMesh( pAiScene, pAiScene->mMeshes[ i ], szModelPath, vpMaterials, i );
	}

	SceneNode* pAssetRootNode = new SceneNode( szModelPath );
	processNode( pScene, pAiScene, pAssetRootNode, pAiScene->mRootNode, vpMeshes );


	//Materials are cloned into the meshes and Meshes are cloned into the model - so these lists can be deleted (assuming the cloning is bug-free ;=) )
	for( int i = 0; i < pAiScene->mNumMaterials; ++i )
		delete vpMaterials[ i ];

	delete[] vpMaterials;

	for( int i = 0; i < pAiScene->mNumMeshes; ++i )
		delete vpMeshes[ i ];

	delete[] vpMeshes;

	m_aiImporter.FreeScene();
	
	return pAssetRootNode;
}

Material* ModelLoader::processMaterial( const aiScene* pAiScene, const aiMaterial* paiMaterial, const String& szModelFolder )
{
	Material* returnMaterial = NULL;

	aiString szName;
	paiMaterial->Get( AI_MATKEY_NAME, szName );
		
	//Determine the Textures defined - and choose the exact type of Engine-Material depending on that
	uint uNumTexturesDiffuse = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_DIFFUSE );
	uint uNumTexturesAmbient = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_AMBIENT );
	uint uNumTexturesNormal = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_NORMALS );
	uint uNumTexturesSpecular = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_SPECULAR );
	uint uNumTexturesGloss = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_SHININESS );

	uint uNumTexturesUnknown = paiMaterial->GetTextureCount( aiTextureType::aiTextureType_UNKNOWN );
	if( uNumTexturesUnknown != 0 )
		LOG( std::string( "WARNING: There are unknown textures defined in Material " ) + std::string( szName.data ) );


	//////////////////////////////////////////////////////////////////////////
	// MAT_Colored
	//////////////////////////////////////////////////////////////////////////
	if(	uNumTexturesDiffuse == 0 && 
				uNumTexturesNormal == 0 && 
				uNumTexturesGloss == 0 && 
				uNumTexturesSpecular == 0 )
	{
		MAT_Colored* pMat = new MAT_Colored();
		pMat->Init();

		returnMaterial = pMat;
	}

	//////////////////////////////////////////////////////////////////////////
	//MAT_TEXTURED
	//////////////////////////////////////////////////////////////////////////
	else if(	uNumTexturesDiffuse >= 1 && 
				uNumTexturesNormal == 0 && 
				uNumTexturesGloss == 0 && 
				uNumTexturesSpecular == 0 )
	{
		MAT_Textured* pMat = new MAT_Textured();
		pMat->Init();

		aiString aiSzDiffTexture;
		//TODO:add mapping, etc.-support
		paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzDiffTexture );
		pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzDiffTexture.data ) );

		returnMaterial = pMat;
	}


	//////////////////////////////////////////////////////////////////////////
	//MAT_TexturedNormal
	//////////////////////////////////////////////////////////////////////////
	else if(	uNumTexturesDiffuse >= 1 && 
				uNumTexturesNormal >= 1 && 
				uNumTexturesGloss == 0 && 
				uNumTexturesSpecular == 0 )
	{
		MAT_TexturedNormal* pMat = new MAT_TexturedNormal();
		pMat->Init();

		aiString aiSzDiffTexture;
		
		//TODO:add mapping, etc.-support
		paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzDiffTexture );
		pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzDiffTexture.data ) );

		aiString aiSzNormTexture;

		//TODO:add mapping, etc.-support
		paiMaterial->GetTexture( aiTextureType_NORMALS, 0, &aiSzNormTexture );
		pMat->GetNormalTexture().SetTexture( szModelFolder + String( aiSzNormTexture.data ) );

		float fBumpIntensity = 1.0f;
		paiMaterial->Get( AI_MATKEY_BUMPSCALING, fBumpIntensity );
		pMat->SetBumpIntensity( fBumpIntensity * fGlobalNormalMod );

		returnMaterial = pMat;
	}


	//////////////////////////////////////////////////////////////////////////
	//MAT_TexturedNormalSpecular
	//////////////////////////////////////////////////////////////////////////
	else if(	uNumTexturesDiffuse >= 1 && 
				uNumTexturesNormal >= 1 && 
				( uNumTexturesGloss >= 1 || uNumTexturesSpecular >= 1 )
			)
	{
		MAT_TexturedNormalSpecular* pMat = new MAT_TexturedNormalSpecular();
		pMat->Init();

		aiString aiSzTexture;

		//TODO:add mapping, etc.-support
		paiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiSzTexture );
		pMat->GetDiffuseTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );

		//TODO:add mapping, etc.-support
		paiMaterial->GetTexture( aiTextureType_NORMALS, 0, &aiSzTexture );
		pMat->GetNormalTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );

		if( uNumTexturesGloss )
		{
			//TODO:add mapping, etc.-support
			paiMaterial->GetTexture( aiTextureType_SHININESS, 0, &aiSzTexture );
			pMat->GetGlossTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );
		}

		if( uNumTexturesSpecular )
		{
			//TODO:add mapping, etc.-support
			paiMaterial->GetTexture( aiTextureType_SPECULAR, 0, &aiSzTexture );
			pMat->GetSpecularTexture().SetTexture( szModelFolder + String( aiSzTexture.data ) );
		}
		
		float fBumpIntensity = 1.0f;
		paiMaterial->Get( AI_MATKEY_BUMPSCALING, fBumpIntensity );
		pMat->SetBumpIntensity( fBumpIntensity * fGlobalNormalMod );

		returnMaterial = pMat;
	}

	

	//////////////////////////////////////////////////////////////////////////
	// MAT_TEST to indicade missing material information
	//////////////////////////////////////////////////////////////////////////
	else
	{
		MAT_Test* pMat = new MAT_Test;
		pMat->Init();

		returnMaterial = pMat;
	}

	//Note: Uncomment to debug with single color
	MAT_Colored* pMat = new MAT_Colored();
	pMat->Init();

	returnMaterial = pMat;
	
	

	//Get Properties that all materials have in common
	aiColor3D diffColor ( 0.0f, 0.0f, 0.0f );
	paiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, diffColor );

	aiColor3D ambColor ( 1.0f, 1.0f, 1.0f );
	paiMaterial->Get( AI_MATKEY_COLOR_AMBIENT, ambColor );

	float fOpacity = 1.0f;
	paiMaterial->Get( AI_MATKEY_OPACITY, fOpacity );

	aiColor3D specColor ( 0.0f, 0.0f, 0.0f );
	paiMaterial->Get( AI_MATKEY_COLOR_SPECULAR, specColor );
	
	float fSpecExponent = 0.0f;
	paiMaterial->Get( AI_MATKEY_SHININESS, fSpecExponent );

	float fGloss = 0.0f;
	paiMaterial->Get( AI_MATKEY_SHININESS_STRENGTH, fGloss );

	returnMaterial->SetColor( glm::vec3( diffColor.r, diffColor.g, diffColor.b ) );
	returnMaterial->SetAmbientReflectivity( glm::vec3( ambColor.r, ambColor.g, ambColor.b ) );
	returnMaterial->SetOpacity( fOpacity );
	returnMaterial->SetSpecularColor(  glm::vec3( specColor.r, specColor.g, specColor.b ) );

	if( fSpecExponent > 255.0f ) 
		fSpecExponent = 255.0f;

	returnMaterial->SetSpecularExponent( fSpecExponent / 255.0f ); //conversion to 0...1
	returnMaterial->SetGlossiness( fGloss 1.0f ); //Hack for now till I figured out a nice way to import that from maya...
	returnMaterial->SetDiffuseReflectivity( glm::vec3( 1.0f, 1.0f, 1.0f ) ); //Hack for now till I figured out a nice way to import that from maya...

	return returnMaterial;
}

Mesh* ModelLoader::processMesh( const aiScene* pAiScene, aiMesh* paiMesh, const String& szModelPath, Material** vpMaterials, uint i, bool assignMaterial  = true  )
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
		stringstream ss;
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
	for( int i = 0; i < paiMesh->GetNumUVChannels() && i < ShaderSemantics::MAX_UV_CHANNELS; ++i )
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


		for( int i = 0; i < paiMesh->mNumVertices; ++i )
		{
			uint8* pVertex = &pVBOdata[ pVertexInfo->GetStride() * i ];

			for( int iVelement = 0; iVelement < vVertexElements.size(); ++iVelement )
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
			for( int iFace = 0; iFace < paiMesh->mNumFaces; ++iFace )
			{
				aiFace& rFace = paiMesh->mFaces[ iFace ];

				for( int iFaceIndex = 0; iFaceIndex < rFace.mNumIndices; ++iFaceIndex )
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
	for( int i = 0; i < paiMesh->mNumVertices; ++i )
		vPositions.push_back( glm::vec3( paiMesh->mVertices[ i ].x, paiMesh->mVertices[ i ].y, paiMesh->mVertices[ i ].z ) );

	pMesh->Init( vPositions );

	//Append the vertex-Info to the mesh. Thereby, the Resource manager gets notified and adds a reference count for the gl-buffers
	pMesh->SetVertexInfo( pVertexInfo );
	
	return pMesh;	
}


void ModelLoader::processNode( SceneManager* pScene, const aiScene* pAiScene, SceneNode* pNode, aiNode* pAiNode, Mesh** vMeshes )
{
	SceneNode* pCurrNode = pNode->createChildSceneNode( String( pAiNode->mName.data ) );
	pCurrNode->setTransform( matFromAiMat( pAiNode->mTransformation ) );
	
	for( int i = 0; i < pAiNode->mNumMeshes; ++i )
	{
		uint uMeshIdx = pAiNode->mMeshes[ i ];
		Mesh* pCurrMesh = new Mesh( *vMeshes[ uMeshIdx ] ); //Clone the mesh to allow multiple meshes defined in one file
		Entity* pEntity = pScene->CreateEntity( std::unique_ptr<Mesh>( pCurrMesh ) );
		pCurrNode->attatchEntity( pEntity );
	}

	for( int i = 0; i < pAiNode->mNumChildren; ++i )
	{
		aiNode* paiChildNode = pAiNode->mChildren[ i ];
		processNode( pScene, pAiScene, pCurrNode, paiChildNode, vMeshes ); //Recursively handle the child nodes
	}
}


glm::mat4 ModelLoader::matFromAiMat( const aiMatrix4x4& mat )
{
	return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
						mat.b1, mat.b2, mat.b3, mat.b4,
						mat.c1, mat.c2, mat.c3, mat.c4,
						mat.d1, mat.d2, mat.d3, mat.d4 );
}

*/
