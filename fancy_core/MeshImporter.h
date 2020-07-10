#pragma once

#include "StaticArray.h"
#include "Shader.h"
#include "Mesh.h"
#include "Ptr.h"
#include "Scene.h"

#include <unordered_map>

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiMaterial;

namespace Fancy {
//---------------------------------------------------------------------------//
  struct Material;
//---------------------------------------------------------------------------//
  class MeshImporter
  {
  public:
    enum ImportOptions
    {
      NONE = 0,
      CALC_TANGENT_SPACE      = 1 << 0,
      TRIANGULATE             = 1 << 1,
      JOIN_IDENTICAL_VERTICES = 1 << 2,
      SPLIT_BY_PRIMITIVE_TYPE = 1 << 3,
      INSTANTIATE_DUPLICATES  = 1 << 4,
      ALL = ~0u
    };

    bool Import(const char* aPath, const StaticArray<VertexShaderAttributeDesc, 16>& someVertexAttributes, SceneData& aResultOut, ImportOptions someImportOptions = ALL);
    
  private:
    bool ProcessNodeRecursive(const aiNode* aNode, const glm::float4x4& aParentTransform, SceneData& aResultOut);
    bool ProcessMeshes(const aiNode* aNode, const glm::float4x4& aTransform, SceneData& aResultOut);

    uint CreateMesh(const aiNode* aNode, aiMesh** someMeshes, uint aMeshCount, SceneData& aResultOut);
    MeshDesc CreateMeshDesc(uint64 anAssimpMeshListHash);

    uint CreateMaterial(const aiMaterial* anAiMaterial, SceneData& aResultOut);
    
    String mySourcePath;
    const aiScene* myScene = nullptr;
    uint64 mySceneFileTimeStamp = 0u;
    VertexInputLayoutProperties myVertexInputLayout;
    StaticArray<VertexShaderAttributeDesc, 16> myVertexAttributes;
    std::unordered_map<const aiMaterial*, uint> myMaterialCache;
    std::unordered_map<uint64, uint> myMeshCache; 
  };
//---------------------------------------------------------------------------//
}
