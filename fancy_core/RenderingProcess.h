#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
class Time;
class RenderOutput;
class RenderContext;

class RenderingProcess
{
  public:
    RenderingProcess() = default;
    virtual ~RenderingProcess() = default;

    virtual void Startup() = 0;
    virtual void Tick(const RenderOutput* anOutput, const Time& aClock) = 0;
};
//---------------------------------------------------------------------------//
}