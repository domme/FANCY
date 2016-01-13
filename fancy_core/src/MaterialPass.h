#ifndef INCLUDE_MATERIALPASS_H
#define INCLUDE_MATERIALPASS_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"

#include "FixedArray.h"
#include "ObjectName.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Serializable.h"
#include "GUID.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  struct MaterialPassDesc
  {
    uint32 m_eFillMode;
    uint32 m_eCullMode;
    uint32 m_eWindingOrder;
    BlendStateDesc m_BlendStateDesc;
    DepthStencilStateDesc m_DepthStencilStateDesc;
    GpuProgramPipelineDesc m_GpuProgramPipelineDesc;
  };
//---------------------------------------------------------------------------//
  class MaterialPass : public StaticManagedHeapObject<MaterialPass>
  {
    friend class MaterialPassInstance;

    public:
      SERIALIZABLE(MaterialPass)

      MaterialPass();
      ~MaterialPass();
      bool operator==(const MaterialPass& _other) const;

      void serialize(IO::Serializer* aSerializer);
      static ObjectName getTypeName() { return _N(MaterialPass); }

      const ObjectName& getName() const { return m_Name; }
      const GpuProgramPipeline* GetProgramPipeline() const { return myProperties.myProgramPipeline; }
      
      FillMode getFillMode() const {return myProperties.m_eFillMode;}
      CullMode getCullMode() const {return myProperties.m_eCullMode;}
      WindingOrder getWindingOrder() const {return myProperties.m_eWindingOrder;}
      const BlendState* getBlendState() const {return myProperties.m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return myProperties.m_pDepthStencilState;}
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name);
      MaterialPassInstance* createMaterialPassInstance(const ObjectName& name, const MaterialPassInstance& _template);
      MaterialPassInstance* getMaterialPassInstance(const ObjectName& aName);
      MaterialPassInstance* getMaterialPassInstance(const uint& anMpiHash);

      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      BlendState* m_pBlendState;
      DepthStencilState* m_pDepthStencilState;
      GpuProgramPipeline* myProgramPipeline;

      ObjectName m_Name;
            
    private:
      // TODO: Re-work the MaterialPass <-> MaterialPassInstance coupling
      std::vector<MaterialPassInstance*> m_vpMaterialPassInstances;

  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H