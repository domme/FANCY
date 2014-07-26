#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace FANCY { namespace Core {  namespace Rendering {

class Renderer : public PLATFORM_DEPENDENT_NAME(Renderer)
{
  public:
    static Renderer& getInstance() { static Renderer instance; return instance; }
    virtual ~Renderer() {}

  protected:
    Renderer() {}
};

} // end of namespace Rendering
} // end of namespace Core
} // end of namespace FANCY

#endif  // INCLUDE_RENDERER_H