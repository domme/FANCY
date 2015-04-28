#ifndef INCLUDE_SCENENODECOMPONENT_H
#define INCLUDE_SCENENODECOMPONENT_H

#include "ObjectName.h"
#include "Serializeable.h"

namespace Fancy { namespace Scene {
//---------------------------------------------------------------------------//
  class SceneNode;
  class SceneRenderDescription;
//---------------------------------------------------------------------------//
  struct SceneNodeComponentDesc
  {
    ObjectNameDesc myTypeName;
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(SceneNodeComponentDesc)
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneNodeComponent : public Serializable
  {
    public:
      SceneNodeComponent(SceneNode* pOwner);
      virtual ~SceneNodeComponent();

      SceneNode* getSceneNode() { return m_pOwner; }
      const SceneNode* getSceneNode() const { return m_pOwner; }
      
      virtual void init() { };
      virtual void update() = 0;
      virtual SceneNodeComponentDescPtr getDescription() const { return nullptr; }
	    virtual void initFromDescription(const SceneNodeComponentDescPtr& someDesc) {};

      virtual void gatherRenderItems(SceneRenderDescription* pRenderDesc) = 0;
      virtual ObjectName getTypeName() const = 0;

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
    static std::shared_ptr<T> create(ArgT arg) { return std::make_shared<T>(arg); }
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Scene

#endif  // INCLUDE_SCENENODECOMPONENT_H