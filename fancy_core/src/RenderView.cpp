#include "RenderView.h"
#include "Renderer.h"
#include "RenderingProcessForward.h"
#include "Fancy.h"
#include "GraphicsWorld.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderView::RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique)
  {
    myRenderOutput = new Rendering::RenderOutput(anAppInstanceHandle);
    myRenderOutput->postInit();

    myGraphicsWorld = new GraphicsWorld();
    
    // Init Rendering process
    switch (aRenderingTechnique)
    {
      case static_cast<uint32>(RenderingTechnique::FORWARD):
        myRenderingProcess = FANCY_NEW(Rendering::RenderingProcessForward, MemoryCategory::General);
        break;
      default:
        ASSERT(false, "Unsupported rendering technique %", aRenderingTechnique);
        break;
    }
  }
//---------------------------------------------------------------------------//
  RenderView::~RenderView()
  {
  }
//---------------------------------------------------------------------------//
}
