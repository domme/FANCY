#pragma once

#include "Common/MathIncludes.h"
#include "Material.h"
#include "Mesh.h"
#include "Assets.h"
#include "Rendering/ResourceHandle.h"

namespace Fancy {
  class Assets;
  //---------------------------------------------------------------------------//
  struct VertexInputLayout;
  struct Mesh;
  //---------------------------------------------------------------------------//
  struct SceneMeshInstance {
    glm::float4x4 myTransform;
    uint          myMeshIndex;
    uint          myMaterialIndex;
  };
  //---------------------------------------------------------------------------//
  struct SceneData {
    eastl::vector< SceneMeshInstance > myInstances;
    eastl::vector< MeshData >          myMeshes;
    eastl::vector< MaterialDesc >      myMaterials;
    VertexInputLayoutProperties        myVertexInputLayoutProperties;
  };
  //---------------------------------------------------------------------------//
  struct Scene {
    Scene( const SceneData & aData );
    eastl::vector< SceneMeshInstance > myInstances;
    eastl::vector< MeshHandle >        myMeshes;
    eastl::vector< MaterialHandle >    myMaterials;
    VertexInputLayoutHandle            myVertexInputLayout;
  };
  //---------------------------------------------------------------------------//

  //---------------------------------------------------------------------------//
}  // namespace Fancy
