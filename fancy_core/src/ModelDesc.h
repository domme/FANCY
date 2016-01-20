#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "SubModel.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  struct ModelDesc
  {
    std::vector<SubModelDesc> mySubmodels;

    bool operator==(const ModelDesc& anOther) const;
    uint64 GetHash() const;
  };
//---------------------------------------------------------------------------//
} }
