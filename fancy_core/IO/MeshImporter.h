#pragma once

#include "Rendering/Shader.h"
#include "Mesh.h"
#include "Scene.h"

#include "EASTL/hash_map.h"
#include "EASTL/string.h"
#include "EASTL/fixed_vector.h"

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

    bool Import(const char* aPath, const eastl::fixed_vector<VertexShaderAttributeDesc, 16>& someVertexAttributes, SceneData& aResultOut, ImportOptions someImportOptions = ALL);
    
  private:
    bool ProcessNodeRecursive(const aiNode* aNode, const glm::float4x4& aParentTransform, SceneData& aResultOut);
    bool ProcessMeshes(const aiNode* aNode, const glm::float4x4& aTransform, SceneData& aResultOut);

    uint CreateMesh(aiMesh** someMeshes, uint aMeshCount, SceneData& aResultOut);
    MeshDesc CreateMeshDesc(uint64 anAssimpMeshListHash);

    uint CreateMaterial(const aiMaterial* anAiMaterial, SceneData& aResultOut);
    
    eastl::string mySourcePath;
    const aiScene* myScene = nullptr;
    uint64 mySceneFileTimeStamp = 0u;
    VertexInputLayoutProperties myVertexInputLayout;
    eastl::fixed_vector<VertexShaderAttributeDesc, 16> myVertexAttributes;
    eastl::hash_map<const aiMaterial*, uint> myMaterialCache;
    eastl::hash_map<uint64, uint> myMeshCache; 
  };
//---------------------------------------------------------------------------//
}
