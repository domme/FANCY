#ifndef INCLUDE_SCENENODECOMPONENT_H
#define INCLUDE_SCENENODECOMPONENT_H

#include "ObjectName.h"
#include "Serializable.h"

namespace Fancy {
  namespace IO {
    class Serializer;
  }
}

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class SceneNode;
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNodeComponent
  {
    friend class SceneNode;

    public:
      SERIALIZABLE(SceneNodeComponent)

      SceneNodeComponent();
      virtual ~SceneNodeComponent();

      uint64 GetHash() const { return 0u; }
      virtual ObjectName getTypeName() const = 0;
      virtual void Serialize(IO::Serializer* aSerializer) = 0;
      
      SceneNode* getSceneNode() { return mySceneNode; }
      const SceneNode* getSceneNode() const { return mySceneNode; }
      
      virtual void init() { };
      virtual void update() = 0;

    protected:
      SceneNode* mySceneNode;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNodeComponent)
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  template<class T>
  class BaseCreator
  {
  public: 
    static T* create() { return FANCY_NEW(T(), MemoryCategory::GENERAL); }
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENENODECOMPONENT_H