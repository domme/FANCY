#include "ModelComponent.h"
#include "SceneRenderDescription.h"
#include "Model.h"
#include "SubModel.h"
#include "Material.h"
#include "MaterialPass.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "SceneNode.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  ModelComponent::ModelComponent(SceneNode* pOwner)
    : SceneNodeComponent(pOwner)
  {

  }
//---------------------------------------------------------------------------//
  ModelComponent::~ModelComponent()
  {

  }
//---------------------------------------------------------------------------//
  void ModelComponent::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&m_pModel, "m_pModel");
  }
//---------------------------------------------------------------------------//
  void ModelComponent::update()
  {
    
  }
//---------------------------------------------------------------------------//
  void ModelComponent::gatherRenderItems( SceneRenderDescription* pRenderDesc )
  {
    if (!m_pModel)
    {
      return;
    }

    const glm::mat4* pWorldMat = &m_pOwner->getTransform().getCachedWorld();

    for (uint32 iPass = 0u; iPass < (uint32) Rendering::EMaterialPass::NUM; ++iPass)
    {
      RenderingItemList& vRenderItems = pRenderDesc->techniqueItemList[iPass];

      const std::vector<SharedPtr<Geometry::SubModel>>& vSubmodels = m_pModel->getSubModelList();
      for (uint32 iSubmodel = 0; iSubmodel < m_pModel->getNumSubModels(); ++iSubmodel)
      {
        const Geometry::SubModel* pSubmodel = vSubmodels[iSubmodel].get();
        Rendering::Material* pMaterial = pSubmodel->getMaterial();

        const Rendering::MaterialPassInstance* pMaterialPassInstance = pMaterial->getPass((Rendering::EMaterialPass)iPass);

        if (!pMaterialPassInstance)
        {
          continue;
        }
          
        const Geometry::Mesh* pMesh = pSubmodel->getMesh();
        const Geometry::GeometryDataList& vGeometryDatas = pMesh->getGeometryDataList();

        for (uint32 iGeometryData = 0u; iGeometryData < vGeometryDatas.size(); ++iGeometryData)
        {
          vRenderItems.push_back(Rendering::RenderingItem(vGeometryDatas[iGeometryData], pMaterialPassInstance, pWorldMat));
        }
      }  // end for submodels
    }  // end for passes
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene