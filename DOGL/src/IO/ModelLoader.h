#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "../Includes.h"

#include "../Geometry/Mesh.h"
#include "../Geometry/Model.h"
#include "ModelLoader_LogStream.h"

#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>

class SceneManager;
class SceneNode;
class Mesh;

class DLLEXPORT ModelLoader
{
public:
	static ModelLoader& GetInstance() { static ModelLoader instance; return instance; }
	~ModelLoader();

	SceneNode* LoadAsset( const String& szModelPath, SceneManager* pScene );
	Mesh* LoadSingleMeshGeometry( const String& szModelPath );

private:
	ModelLoader();
	void processNode( SceneManager* pScene, const aiScene* pAiScene, SceneNode* pNode, aiNode* pAiNode, Mesh** vMeshes );
	Mesh* processMesh( const aiScene* pAiScene, aiMesh* paiMesh, const String& szModelPath, Material** vpMaterials, uint i, bool assignMaterial = true );
	Material* processMaterial( const aiScene* pAiScene, const aiMaterial* paiMaterial, const String& szModelFolder );

	glm::mat4 matFromAiMat( const aiMatrix4x4& mat );

	Assimp::Importer		m_aiImporter;
	ModelLoader_LogStream	m_clLogStream;
	
};


#endif