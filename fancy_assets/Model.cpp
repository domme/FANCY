#include "Model.h"
#include "Material.h"
#include <fancy_core/Mesh.h>

using namespace Fancy;

Model::Model()
  : myMesh(nullptr)
  , myMaterial(nullptr)
{

}

ModelDesc Model::GetDescription() const
{
  ModelDesc desc;
  if (myMesh)
    desc.myMesh = myMesh->GetDescription();

  if (myMaterial)
    desc.myMaterial = myMaterial->GetDescription();

  return desc;
}
