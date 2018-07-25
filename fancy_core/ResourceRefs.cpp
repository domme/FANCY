#include "ResourceRefs.h"

namespace Fancy
{
  namespace ResourceRef
  {
    const char* ToString(TextureRef aRef)
    {
      switch(aRef)
      {
      case TextureRef::DEFAULT_DIFFUSE: return "DEFAULT_DIFFUSE";
      case TextureRef::DEFAULT_NORMAL: return "DEFAULT_NORMAL";
      case TextureRef::DEFAULT_SPECULAR: return "DEFAULT_SPECULAR";
      case TextureRef::DEFAULT_BACKBUFFER: 
      case TextureRef::DEFAULT_DEPTHSTENCILBUFFER: break;
      case TextureRef::NUM: break;
      default: break;
      }
    }

    const char* GetName(BufferRef aRef)
    {
    }

    const char* GetName(MeshRef aRef)
    {
    }
  }
}