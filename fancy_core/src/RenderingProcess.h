#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy{
class GraphicsWorld;
class Time;
}

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {

class RenderOutput;
class RenderContext;

class DLLEXPORT RenderingProcess
{
  public:
    RenderingProcess();
    virtual ~RenderingProcess();

    virtual void Startup() = 0;
    virtual void Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) = 0;
};
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(RenderingProcess)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering