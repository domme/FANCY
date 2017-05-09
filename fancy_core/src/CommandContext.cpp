#include "CommandContext.h"
#include "RenderContext.h"
#include "ComputeContext.h"
#include "CommandListType.h"
#include <list>

#include "RenderContextDX12.h"
#include "ComputeContextDX12.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace {
    
  }
//---------------------------------------------------------------------------//
  CommandContext::CommandContext(CommandListType aType)
    : myCommandListType(aType)
  {

  }
//---------------------------------------------------------------------------//
  CommandContext* CommandContextPool::AllocateContext(CommandListType aType)
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

    // TODO REFACTOR: Make some platform-specific factory for concrete Contexts (DX12/Vk/...). 
    // Maybe the RenderCore could be a good place - also to handle the pools in a centralized way?
    if (aType == CommandListType::Graphics)
      contextPool.push_back(std::make_unique<DX12::RenderContextDX12>());
    else
      contextPool.push_back(std::make_unique<DX12::ComputeContextDX12>());

    return contextPool.back().get();
  }
//---------------------------------------------------------------------------//
  void CommandContextPool::FreeContext(CommandContext* aContext)
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
