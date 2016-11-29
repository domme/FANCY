#pragma once

#if defined (RENDERER_DX12)

namespace Fancy{ namespace IO{
class Serializer;
}}

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

      class GpuProgramResourceDX12
      {
      public:
        GpuProgramResourceDX12();
        ~GpuProgramResourceDX12();

        void Serialize(IO::Serializer* aSerializer);
      };

    }
  }
}

#endif

