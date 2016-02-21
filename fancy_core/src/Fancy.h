#ifndef INCLUDE_ENGINECOMMON_H
#define INCLUDE_ENGINECOMMON_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Scene { 
  class Scene;
  typedef std::shared_ptr<Scene> ScenePtr;
} }

namespace Fancy { namespace Rendering {
  class RenderingProcess;
  class Renderer;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
    DLLEXPORT bool Init(void* aNativeWindowHandle);
    DLLEXPORT void Shutdown();
    DLLEXPORT void SetCurrentScene(const Scene::ScenePtr& _pScene);
    DLLEXPORT void Startup();
    DLLEXPORT void Update(double _dt);
    DLLEXPORT void SetRenderingProcess(Rendering::RenderingProcess* _pRenderingProcess);
    DLLEXPORT void SetWindowSize(uint32 _uWidth, uint32 _uHeight);

    Rendering::Renderer* GetRenderer();
    Rendering::RenderingProcess* GetRenderingProcess();
    const Scene::ScenePtr& GetCurrentScene();
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
