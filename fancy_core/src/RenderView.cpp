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
    SAFE_DELETE(myRenderingProcess);

    myRenderOutput.reset();
    myGraphicsWorld.reset();
  }
//---------------------------------------------------------------------------//
  RenderWindow* RenderView::GetRenderWindow() const
  {
    return myRenderOutput->GetWindow();
  }
//---------------------------------------------------------------------------//
  void RenderView::Startup() const
  {
    myRenderingProcess->Startup();
    myGraphicsWorld->Startup();
  }
//---------------------------------------------------------------------------//
  void RenderView::BeginFrame() const
  {
    myRenderOutput->BeginFrame();
  }
//---------------------------------------------------------------------------//
  void RenderView::Tick(const Time& aClock) const
  {
    myGraphicsWorld->Tick(aClock);
    myRenderingProcess->Tick(myGraphicsWorld.get(), myRenderOutput.get(), aClock);
  }
//---------------------------------------------------------------------------//
  void RenderView::EndFrame() const
  {
    myRenderOutput->EndFrame();
  }
//---------------------------------------------------------------------------//
}
