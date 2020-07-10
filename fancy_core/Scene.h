#pragma once

#include "MathIncludes.h"
#include "Ptr.h"
#include "DynamicArray.h"
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
    DynamicArray<SceneMeshInstance> myInstances;
    DynamicArray<MeshData> myMeshes;
    DynamicArray<MaterialDesc> myMaterials;
    VertexInputLayoutProperties myVertexInputLayoutProperties;
  };
//---------------------------------------------------------------------------//
  struct Scene
  {
    Scene(const SceneData& aData);
    DynamicArray<SceneMeshInstance> myInstances;
    DynamicArray<SharedPtr<Mesh>> myMeshes;
    DynamicArray<SharedPtr<Material>> myMaterials;
    SharedPtr<VertexInputLayout> myVertexInputLayout;
  };
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
}
