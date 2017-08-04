#ifndef INCLUDE_SCENE_H
#define INCLUDE_SCENE_H

#include "FancyCorePrerequisites.h"
#include "FixedArray.h"

namespace Fancy { namespace Scene {
  class SceneNodeComponent;
  //---------------------------------------------------------------------------//
  // Forward declarations:
  class SceneNode;
  class CameraComponent;
  class SceneRenderDescription;
  class LightComponent;
  class ModelComponent;
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  const uint32 kMaxNumLights = 512;
  const uint32 kMaxNumModels = 2048;

  typedef FixedArray<const LightComponent*, kMaxNumLights> LightList;
  typedef FixedArray<const ModelComponent*, kMaxNumModels> ModelList;

  class DLLEXPORT Scene
  {
    public:
      Scene();
      ~Scene();

      SceneNode* getRootNode() const {return m_pRootNode;}

      void update(float _dt);
      void startup();
      void setActiveCamera(CameraComponent* _pCamera) {m_pActiveCamera = _pCamera;}
      CameraComponent* getActiveCamera() const {return m_pActiveCamera;}

      const LightList& getCachedLights() const { return myLights; }
      const ModelList& getCachedModels() const { return myModels; }

    private:
      LightList myLights;
      ModelList myModels;

      SceneNode* m_pRootNode;
      CameraComponent* m_pActiveCamera;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(Scene)
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENE_H
