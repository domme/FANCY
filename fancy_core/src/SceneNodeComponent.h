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
  class SceneRenderDescription;
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNodeComponent
  {
    public:
      SERIALIZABLE(SceneNodeComponent)

      SceneNodeComponent(SceneNode* pOwner);
      virtual ~SceneNodeComponent();

      uint64 GetHash() const { return 0u; }
      virtual ObjectName getTypeName() const = 0;
      virtual void serialize(IO::Serializer* aSerializer) = 0;
      
      SceneNode* getSceneNode() { return m_pOwner; }
      const SceneNode* getSceneNode() const { return m_pOwner; }
      
      virtual void init() { };
      virtual void update() = 0;

      virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) = 0;

    protected:
      SceneNode* m_pOwner;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNodeComponent)
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  template<class T, class ArgT>
  class BaseCreator
  {
  public: 
    static T* create(ArgT arg) { return FANCY_NEW(T(arg), MemoryCategory::GENERAL); }
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENENODECOMPONENT_H