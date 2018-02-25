#pragma once

#include <fancy_core/FancyCorePrerequisites.h>

namespace Fancy {
//---------------------------------------------------------------------------//
  class SubModel;
//---------------------------------------------------------------------------//
  class Model
  {
    public:
      DynamicArray<SharedPtr<SubModel>> mySubModels;
  };
//---------------------------------------------------------------------------//
}