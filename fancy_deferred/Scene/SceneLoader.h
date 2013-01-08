#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <Includes.h>
#include <IO/ModelLoader.h>
#include <Rendering/Materials/Material.h>

#include "SceneNode.h"
#include "SceneManager.h"

class DLLEXPORT SceneLoader
{
public:
	static SceneLoader& GetInstance() { static SceneLoader clInstance; return clInstance; }
	~SceneLoader(void);
	SceneNode* LoadAsset( const String& szModelPath, SceneManager* pScene );
	Material* processMaterial( const aiScene* pAiScene, const aiMaterial* paiMaterial, const String& szModelFolder );
	void processNode( SceneManager* pScene, const aiScene* pAiScene, SceneNode* pNode, aiNode* pAiNode, Mesh** vMeshes );

private:
	SceneLoader(void);



};

#endif

