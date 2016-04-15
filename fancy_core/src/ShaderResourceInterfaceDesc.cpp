
#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "ShaderResourceInterfaceDesc.h"

#include "RenderContext.h"
#include "MaterialPassInstance.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  /*void ShaderResourceInterface::Bind(RenderContext* aRenderContext, const ResourceBindingDataSource& aDataSource) const
  {
    uint32 registerIndex = 0u;
    for (const Element& elem : myElements)
    {
      switch(elem.myType)
      {
        case ElementType::Constant: 
          ASSERT(false, "SRI-Constants not implemented yet");
          break;
        case ElementType::SingleBinding: 
          switch(elem.myResourceType)
          {
            case ResourceType::ConstantBuffer: 
              aRenderContext->SetConstantBuffer(aDataSource.myConstantBuffers[elem.myBindingSlot], registerIndex);
              break;
            case ResourceType::BufferOrTexture: 
              
              break;
            case ResourceType::BufferOrTextureRW: break;
            case ResourceType::Sampler: break;
            default: break;
          }
          break;
        case ElementType::RangeBinding: break;
        default: break;
      }

      ++registerIndex;
    }
  } */
//---------------------------------------------------------------------------//
} }  // Fancy::Rendering