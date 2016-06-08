#include "CommandContext.h"
#include "RenderContext.h"
#include "ComputeContext.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace {
    std::vector<std::unique_ptr<CommandContext>> locRenderContextPool;
    std::vector<std::unique_ptr<CommandContext>> locComputeContextPool;
    std::list<CommandContext*> locAvailableRenderContexts;
    std::list<CommandContext*> locAvailableComputeContexts;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  CommandContext* CommandContext::AllocateContext(CommandListType aType)
  {
    ASSERT(aType == CommandListType::Graphics || aType == CommandListType::Compute, 
      "CommandContext type % not implemented", (uint) aType);

    std::vector<std::unique_ptr<CommandContext>>& contextPool = 
      aType == CommandListType::Graphics ? locRenderContextPool : locComputeContextPool;

    std::list<CommandContext*>& availableContextList =
      aType == CommandListType::Graphics ? locAvailableRenderContexts : locAvailableComputeContexts;

    if (!availableContextList.empty())
    {
      CommandContext* context = availableContextList.front();
      context->Reset();
      availableContextList.pop_front();
      return context;
    }

    if (aType == CommandListType::Graphics)
      contextPool.push_back(std::make_unique<RenderContext>());
    else
      contextPool.push_back(std::make_unique<ComputeContext>());

    return contextPool.back().get();
  }
//---------------------------------------------------------------------------//
  void CommandContext::FreeContext(CommandContext* aContext)
  {
    CommandListType type = aContext->GetType();

    ASSERT(type == CommandListType::Graphics || type == CommandListType::Compute,
      "CommandContext type % not implemented", (uint)type);

    std::list<CommandContext*>& availableContextList =
      type == CommandListType::Graphics ? locAvailableRenderContexts : locAvailableComputeContexts;

      if (std::find(availableContextList.begin(), availableContextList.end(), aContext)
        != availableContextList.end())
        return;

      availableContextList.push_back(aContext);
  }
//---------------------------------------------------------------------------//  
} }