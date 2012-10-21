#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "../Includes.h"

#include "../Geometry/Mesh.h"

#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ModelLoader_LogStream.h"

class Mesh;

class DLLEXPORT ModelLoader
{
public:
	static ModelLoader& GetInstance() { static ModelLoader instance; return instance; }
	~ModelLoader();

	Mesh* LoadSingleMeshGeometry( const String& szModelPath );
	Mesh* ProcessMesh( const aiScene* pAiScene, aiMesh* paiMesh, const String& szModelPath, Material** vpMaterials, uint i, bool assignMaterial = true );
	Assimp::Importer&	GetImporter() { return m_aiImporter; }
	
	glm::mat4 MatFromAiMat( const aiMatrix4x4& mat );	

private:
	ModelLoader();
	
	
	

	Assimp::Importer		m_aiImporter;
	ModelLoader_LogStream	m_clLogStream;
	
};


#endif