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
} }
