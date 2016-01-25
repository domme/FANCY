#pragma once

#if defined (RENDERER_DX12)
#include "TextureSamplerDesc.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class TextureSamplerDX12
  {
  public:
    TextureSamplerDX12();
    ~TextureSamplerDX12();
    bool operator==(const TextureSamplerDesc& aDesc) const { return myProperties == aDesc; }

    const ObjectName& getName() const { return myName; }

    const TextureSamplerDesc& getProperties() const { return myProperties; }

    const TextureSamplerDesc& GetDescription() const { return myProperties; }
    void SetFromDescription(const TextureSamplerDesc& aDesc);
    void create(const ObjectName& rName, const TextureSamplerDesc& rProperties);

  protected:
    ObjectName myName;
    TextureSamplerDesc myProperties;
  };
//---------------------------------------------------------------------------//
} } }

#endif

