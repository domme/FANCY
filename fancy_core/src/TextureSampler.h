#pragma once

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"
#include "Serializable.h"
#include "TextureSamplerDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class TextureSampler
  {
  public:
    SERIALIZABLE_RESOURCE(TextureSampler)

    TextureSampler() = default;
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
} } // end of namespace Fancy::Rendering