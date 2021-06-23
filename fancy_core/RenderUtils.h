#pragma once

#include "FancyCoreDefines.h"
#include "RendererPrerequisites.h"
#include "RenderEnums.h"

#include <cassert>

namespace Fancy
{
  namespace RenderUtils
  {
    uint GetNumDescriptors(GlobalResourceType aType, const RenderPlatformProperties& someProperties)
    {
      switch (aType) {
      case GLOBAL_RESOURCE_TEXTURE_1D:
      case GLOBAL_RESOURCE_TEXTURE_1D_UINT:
      case GLOBAL_RESOURCE_TEXTURE_1D_INT:
      case GLOBAL_RESOURCE_RWTEXTURE_1D:
      case GLOBAL_RESOURCE_RWTEXTURE_1D_UINT:
      case GLOBAL_RESOURCE_RWTEXTURE_1D_INT:
        return someProperties.myNumGlobalTextures1D;
      case GLOBAL_RESOURCE_TEXTURE_2D:
      case GLOBAL_RESOURCE_TEXTURE_2D_UINT:
      case GLOBAL_RESOURCE_TEXTURE_2D_INT:
      case GLOBAL_RESOURCE_RWTEXTURE_2D:
      case GLOBAL_RESOURCE_RWTEXTURE_2D_UINT:
      case GLOBAL_RESOURCE_RWTEXTURE_2D_INT:
        return someProperties.myNumGlobalTextures2D;
      case GLOBAL_RESOURCE_TEXTURE_3D:
      case GLOBAL_RESOURCE_TEXTURE_3D_UINT:
      case GLOBAL_RESOURCE_TEXTURE_3D_INT:
      case GLOBAL_RESOURCE_RWTEXTURE_3D:
      case GLOBAL_RESOURCE_RWTEXTURE_3D_UINT:
      case GLOBAL_RESOURCE_RWTEXTURE_3D_INT:
        return someProperties.myNumGlobalTextures3D;
      case GLOBAL_RESOURCE_TEXTURE_CUBE:
      case GLOBAL_RESOURCE_TEXTURE_CUBE_UINT:
      case GLOBAL_RESOURCE_TEXTURE_CUBE_INT:
        return someProperties.myNumGlobalTexturesCube;
      case GLOBAL_RESOURCE_BUFFER:
      case GLOBAL_RESOURCE_RWBUFFER:
        return someProperties.myNumGlobalBuffers;
      case GLOBAL_RESOURCE_SAMPLER:
        return someProperties.myNumGlobalSamplers;
      default: assert(false); return 0;
      }


      

    }
  }
}
