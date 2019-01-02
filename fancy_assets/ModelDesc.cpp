#include "fancy_assets_precompile.h"

#include "ModelDesc.h"

using namespace Fancy;

uint64 ModelDesc::GetHash() const
{
  uint64 hash;
  MathUtil::hash_combine(hash, myMaterial.GetHash());
  MathUtil::hash_combine(hash, myMesh.GetHash());
  return hash;
}