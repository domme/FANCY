#include "RenderView.h"
#include "Renderer.h"
#include "RenderingProcessForward.h"
#include "Fancy.h"
#include "GraphicsWorld.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderView::RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique, const SharedPtr<GraphicsWorld>& aWorld)
    : myGraphicsWorld(aWorld)
  {
    myRenderOutput = new Rendering::RenderOutput(anAppInstanceHandle);
    myRenderOutput->postInit();
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
  void RenderView::Startup()
  {
    myRenderingProcess->Startup();
    myGraphicsWorld->Startup();
  }
//---------------------------------------------------------------------------//
  void RenderView::Tick(const Time& aClock)
  {
    myGraphicsWorld->Tick(aClock);

    myRenderOutput->beginFrame();
    myRenderingProcess->Tick();
    myRenderOutput->endFrame();
  }
//---------------------------------------------------------------------------//
}
