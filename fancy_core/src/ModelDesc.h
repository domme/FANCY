#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "SubModel.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct ModelDesc : public DescriptionBase
  {
    ~ModelDesc() override {}
    
    bool operator==(const ModelDesc& anOther) const;
    uint64 GetHash() const override;
    
    ObjectName GetTypeName() const override { return _N(Model); }
    void Serialize(IO::Serializer* aSerializer) override;
    bool IsEmpty() const override;

    std::vector<SubModelDesc> mySubmodels;
  };
//---------------------------------------------------------------------------//
} }
