#pragma once

#if defined (RENDERER_DX12)

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

      class TextureSamplerDX12
      {
      public:
        TextureSamplerDX12();
        ~TextureSamplerDX12();

        const ObjectName& getName() const { return myName; }

        const TextureSamplerProperties& getProperties() const { return myProperties; }
        void create(const ObjectName& rName, const TextureSamplerProperties& rProperties);

      protected:
        ObjectName myName;
        TextureSamplerProperties myProperties;
      };
    }
  }
}

#endif

