#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "MaterialPassInstanceDesc.h"
#include "FixedArray.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class EMaterialParameterSemantic
  {
    DIFFUSE_REFLECTIVITY = 0,
    SPECULAR_REFLECTIVITY,
    SPECULAR_POWER,
    OPACITY,

    NUM
  };
//---------------------------------------------------------------------------//
  struct MaterialDesc : public DescriptionBase
  {
    MaterialDesc();
    ~MaterialDesc() override {}

    bool operator==(const MaterialDesc& anOther) const;
    uint64 GetHash() const override;
    void Serialize(IO::Serializer* aSerializer) override;
    ObjectName GetTypeName() const override { return _N(Material); }
    bool IsEmpty() const override;

    float myParameters [(uint32)EMaterialParameterSemantic::NUM];
    MaterialPassInstanceDesc myPasses [(uint32)EMaterialPass::NUM];
  };
//---------------------------------------------------------------------------//
} }

