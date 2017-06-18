#include "RenderView.h"
#include "RenderingProcessForward.h"
#include "Fancy.h"
#include "GraphicsWorld.h"
#include "TimeManager.h"
#include "RenderCore.h"
#include "RenderOutput.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderView::RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique, const SharedPtr<GraphicsWorld>& aWorld)
    : myGraphicsWorld(aWorld)
  {
    myRenderOutput = Rendering::RenderCore::CreateRenderOutput(anAppInstanceHandle);
    myRenderOutput->PrepareForFirstFrame();

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
  RenderWindow* RenderView::GetRenderWindow() const
  {
    return myRenderOutput->GetWindow();
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

    myRenderOutput->BeginFrame();
    myRenderingProcess->Tick(myGraphicsWorld.get(), myRenderOutput.get(), aClock);
    myRenderOutput->EndFrame();
  }
//---------------------------------------------------------------------------//
}
