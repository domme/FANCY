#ifndef INCLUDE_ENGINECOMMON_H
#define INCLUDE_ENGINECOMMON_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Scene { 
  class Scene;
  typedef std::shared_ptr<Scene> ScenePtr;
} }

namespace Fancy { namespace Rendering {
  class RenderingProcess;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class DLLEXPORT EngineCommon
  {
  //---------------------------------------------------------------------------//
  public:
    ~EngineCommon();

    static bool initEngine();
    static void shutdownEngine();

    static void setCurrentScene(const Scene::ScenePtr& _pScene);
    static const Scene::ScenePtr& getCurrentScene();
    static void startup();
    static void update(double _dt);
    static void setRenderingProcess(Rendering::RenderingProcess* _pRenderingProcess);
    static void setWindowSize(uint32 _uWidth, uint32 _uHeight);

  private:
    EngineCommon();

    static void initComponentSubsystem();
    static void initRenderingSubsystem();
    static void initIOsubsystem();

    static Scene::ScenePtr m_pCurrScene;
    static Rendering::RenderingProcess* m_pRenderingProcess;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
