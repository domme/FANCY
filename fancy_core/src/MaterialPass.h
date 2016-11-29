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
#include "GpuProgramDesc.h"
#include "MaterialPassDesc.h"

namespace Fancy { namespace IO {
  class Serializer;
} }

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class MaterialPassInstance;
//---------------------------------------------------------------------------//
  class MaterialPass : public StaticManagedHeapObject<MaterialPass>
  {
    friend class MaterialPassInstance;

    public:
      SERIALIZABLE(MaterialPass)

      MaterialPass();
      ~MaterialPass();
      bool operator==(const MaterialPass& _other) const;
      bool operator==(const MaterialPassDesc& aDesc) const;

      MaterialPassDesc GetDescription() const;
      void SetFromDescription(const MaterialPassDesc& aDesc);

      void Serialize(IO::Serializer* aSerializer);
      static ObjectName getTypeName() { return _N(MaterialPass); }
      uint64 GetHash() const { return GetDescription().GetHash(); }

      const GpuProgramPipeline* GetProgramPipeline() const { return myProgramPipeline.get(); }
      
      FillMode getFillMode() const {return m_eFillMode;}
      CullMode getCullMode() const {return m_eCullMode;}
      WindingOrder getWindingOrder() const {return m_eWindingOrder;}
      const BlendState* getBlendState() const {return m_pBlendState;}
      const DepthStencilState* getDepthStencilState() const {return m_pDepthStencilState;}
      
      FillMode m_eFillMode;
      CullMode m_eCullMode;
      WindingOrder m_eWindingOrder;
      BlendState* m_pBlendState;
      DepthStencilState* m_pDepthStencilState;
      SharedPtr<GpuProgramPipeline> myProgramPipeline;
  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_MATERIALPASS_H