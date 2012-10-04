#include "SceneLoader.h"


#include "../Rendering/Materials/MAT_Colored.h"
#include "../Rendering/Materials/MAT_Test.h"
#include "../Rendering/Materials/MAT_Textured.h"
#include "../Rendering/Materials/MAT_TexturedNormal.h"
#include "../Rendering/Materials/MAT_TexturedNormalSpecular.h"

static const float fGlobalNormalMod = 1.0f;

SceneLoader::SceneLoader(void)
{

}


SceneLoader::~SceneLoader(void)
{
}

SceneNode* SceneLoader::LoadAsset( const String& szModelPath, SceneManager* pScene )
{
	String szAbsPath = PathService::convertToAbsPath( szModelPath );
	String szModelFolder = PathService::GetContainingFolder( szModelPath );

	ModelLoader& rModelLoader = ModelLoader::GetInstance();
	Assimp::Importer& clImporter = rModelLoader.GetImporter();

	const aiScene* pAiScene = clImporter.ReadFile( szAbsPath, aiProcess_CalcTangentSpace |
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
		vpMeshes[ i ] = rModelLoader.ProcessMesh( pAiScene, pAiScene->mMeshes[ i ], szModelPath, vpMaterials, i );
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

	clImporter.FreeScene();
	
	return pAssetRootNode;
}

void SceneLoader::processNode( SceneManager* pScene, const aiScene* pAiScene, SceneNode* pNode, aiNode* pAiNode, Mesh** vMeshes )
{
	ModelLoader& rModelLoader = ModelLoader::GetInstance();

	SceneNode* pCurrNode = pNode->createChildSceneNode( String( pAiNode->mName.data ) );
	pCurrNode->setTransform( rModelLoader.MatFromAiMat( pAiNode->mTransformation ) );

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



Material* SceneLoader::processMaterial( const aiScene* pAiScene, const aiMaterial* paiMaterial, const String& szModelFolder )
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

	//MAT_Colored* pMat = new MAT_Colored();
	//pMat->Init();

	//returnMaterial = pMat;



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
	returnMaterial->SetGlossiness(  1.0f ); //Hack for now till I figured out a nice way to import that from maya...
	returnMaterial->SetDiffuseReflectivity( glm::vec3( 1.0f, 1.0f, 1.0f ) ); //Hack for now till I figured out a nice way to import that from maya...

	return returnMaterial;
}


