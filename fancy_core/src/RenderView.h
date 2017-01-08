#pragma once
#include "ScopedPtr.h"

namespace Fancy { namespace Scene {
  class Scene;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderView
  {
  private:

    ScopedPtr<Scene::Scene> myScene;

  };
//---------------------------------------------------------------------------//
}
