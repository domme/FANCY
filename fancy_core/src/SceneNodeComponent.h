#ifndef INCLUDE_SCENENODECOMPONENT_H
#define INCLUDE_SCENENODECOMPONENT_H

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNodeComponent
  {
    public:
      SceneNodeComponent();
      virtual ~SceneNodeComponent();

      virtual void update(float fDt) = 0;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENENODECOMPONENT_H