#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct ModelDesc;
  struct SubModelDesc;
  struct MeshDesc;
  //---------------------------------------------------------------------------//
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class GraphicsWorld
  {
    public:
      GraphicsWorld();

      SharedPtr<Geometry::Mesh> GetMesh(uint64 aVertexIndexHash);
    
      SharedPtr<Geometry::Mesh> CreateMesh(const Geometry::MeshDesc& aDesc, 
          const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas, 
          const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices);

      // SharedPtr<Geometry::SubModel> CreateSubModel(const Geometry::SubModelDesc& aDesc);
      // SharedPtr<Geometry::Model> CreateModel(const Geometry::ModelDesc& aDesc);

    private:
      std::map<uint64, SharedPtr<Geometry::Mesh>> myCachedMeshes;
  };
//---------------------------------------------------------------------------//
} 