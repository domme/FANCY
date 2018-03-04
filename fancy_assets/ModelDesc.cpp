#include "ModelDesc.h"
#include "fancy_core/MathUtil.h"

using namespace Fancy;

uint64 ModelDesc::GetHash() const
{
  uint64 hash;
  MathUtil::hash_combine(hash, myMaterial.GetHash());
  MathUtil::hash_combine(hash, myMesh.GetHash());
  return hash;
}