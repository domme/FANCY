#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy{
class GraphicsWorld;
class Time;
}

//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {

class RenderOutput;
class RenderContext;

class RenderingProcess
{
  public:
    RenderingProcess() = default;
    virtual ~RenderingProcess() = default;

    virtual void Startup() = 0;
    virtual void Tick(const GraphicsWorld* aWorld, const RenderOutput* anOutput, const Time& aClock) = 0;
};
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering