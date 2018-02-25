#pragma once

#include "FancyCorePrerequisites.h"
#include "TextureSamplerDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSampler
  {
  public:
    virtual ~TextureSampler() = default;

    TextureSamplerDesc GetDescription() const { return myDescription; }

    void SetFromDescription(const TextureSamplerDesc& aDesc)
    {
      if (aDesc == myDescription && IsCreated())
        return;
       
      myDescription = aDesc;
      Create();
    }

  protected:
    TextureSamplerDesc myDescription;

    virtual void Create() = 0;
    virtual bool IsCreated() = 0;
  };
//---------------------------------------------------------------------------//
}