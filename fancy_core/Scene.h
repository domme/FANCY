#pragma once

#include "MathIncludes.h"
#include "Ptr.h"
#include "Material.h"
#include "Mesh.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct VertexInputLayout;
  struct Mesh;
//---------------------------------------------------------------------------//
  struct SceneMeshInstance
  {
    glm::float4x4 myTransform;
    uint myMeshIndex;
    uint myMaterialIndex;
  };
//---------------------------------------------------------------------------//
  struct SceneData
  {
    eastl::vector<SceneMeshInstance> myInstances;
    eastl::vector<MeshData> myMeshes;
    eastl::vector<MaterialDesc> myMaterials;
    VertexInputLayoutProperties myVertexInputLayoutProperties;
  };
//---------------------------------------------------------------------------//
  struct Scene
  {
    Scene(const SceneData& aData);
    eastl::vector<SceneMeshInstance> myInstances;
    eastl::vector<SharedPtr<Mesh>> myMeshes;
    eastl::vector<SharedPtr<Material>> myMaterials;
    SharedPtr<VertexInputLayout> myVertexInputLayout;
  };
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
}
