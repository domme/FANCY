
#include "FancyCorePrerequisites.h"
#include "RenderContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace {
    std::vector<std::unique_ptr<RenderContext>> locRenderContextPool;
    std::list<RenderContext*> locAvailableRenderContexts;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
RenderContext* RenderContext::AllocateContext()
{
  if (!locAvailableRenderContexts.empty())
  {
    RenderContext* context = locAvailableRenderContexts.front();
    context->Reset();
    locAvailableRenderContexts.pop_front();
    return context;
  }

  locRenderContextPool.push_back(std::make_unique<RenderContext>());
  return locRenderContextPool.back().get();
}
//---------------------------------------------------------------------------//
void RenderContext::FreeContext(RenderContext* aContext)
{
  if (std::find(locAvailableRenderContexts.begin(), locAvailableRenderContexts.end(), aContext)
    != locAvailableRenderContexts.end())
    return;

  locAvailableRenderContexts.push_back(aContext);
}
//---------------------------------------------------------------------------//  
} }

