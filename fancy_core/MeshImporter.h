#pragma once

#include "DynamicArray.h"
#include "MathIncludes.h"
#include "StaticArray.h"
#include "Shader.h"
#include "Mesh.h"
#include "Ptr.h"

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

    struct ImportResult
    {
      DynamicArray<glm::float4x4> myTransforms;
      DynamicArray<SharedPtr<Mesh>> myMeshes;
      DynamicArray<SharedPtr<Material>> myMaterials;
      SharedPtr<VertexInputLayout> myVertexInputLayout;
    };

    bool Import(const char* aPath, const StaticArray<VertexShaderAttributeDesc, 16>& someVertexAttributes, ImportResult& aResultOut, ImportOptions someImportOptions = ALL);
    
  private:
    bool ProcessNodeRecursive(const aiNode* aNode, const glm::float4x4& aParentTransform, ImportResult& aResultOut);
    bool ProcessMeshes(const aiNode* aNode, const glm::float4x4& aTransform, ImportResult& aResultOut);

    SharedPtr<Mesh> CreateMesh(const aiNode* aNode, aiMesh** someMeshes, uint aMeshCount);
    MeshDesc CreateMeshDesc(uint64 anAssimpMeshListHash);

    SharedPtr<Material> CreateMaterial(const aiMaterial* anAiMaterial);
    
    String mySourcePath;
    const aiScene* myScene = nullptr;
    uint64 mySceneFileTimeStamp = 0u;
    SharedPtr<VertexInputLayout> myVertexInputLayout;
    StaticArray<VertexShaderAttributeDesc, 16> myVertexAttributes;
    std::unordered_map<const aiMaterial*, SharedPtr<Material>> myMaterialCache;
    std::unordered_map<uint64, SharedPtr<Mesh>> myMeshCache;
  };
//---------------------------------------------------------------------------//
}
