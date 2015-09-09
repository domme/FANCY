#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "TextureDX12.h"

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

      TextureDX12::TextureDX12()
      {
      }


      TextureDX12::~TextureDX12()
      {
      }

      void TextureDX12::create(const TextureDesc& clDeclaration, CreationMethod eCreationMethod)
      {

      }

      void TextureDX12::setPixelData(void* pData, uint uDataSizeBytes, glm::u32vec3 rectPosOffset, glm::u32vec3 rectDimensions)
      {

      }

      void* TextureDX12::lock(GpuResoruceLockOption option)
      {
        return nullptr;
      }

      void TextureDX12::unlock()
      {
      }
    }
  }
}

#endif
