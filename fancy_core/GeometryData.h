#pragma once

#include "FancyCoreDefines.h"
#include "GpuBuffer.h"
#include "Ptr.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GeometryData 
  {
    public:
      const VertexInputLayout* GetVertexInputLayout() const { return myVertexInputLayout.get(); }
      const GpuBuffer* GetVertexBuffer() const {return myVertexBuffer.get();}
      const GpuBuffer* GetIndexBuffer() const {return myIndexBuffer.get();}
      uint64 GetNumVertices() const {return myVertexBuffer ? myVertexBuffer->GetProperties().myNumElements : 0u; }
      uint64 GetNumIndices() const {return myIndexBuffer ? myIndexBuffer->GetProperties().myNumElements : 0u; }
      
      void SetVertexBuffer(SharedPtr<GpuBuffer>& aVertexBuffer) {myVertexBuffer = aVertexBuffer;}
      void SetIndexBuffer(SharedPtr<GpuBuffer>& anIndexBuffer) {myIndexBuffer = anIndexBuffer;}
      void SetVertexLayout(const SharedPtr<VertexInputLayout>& aVertexLayout) { myVertexInputLayout = aVertexLayout; }

    protected:
      SharedPtr<VertexInputLayout> myVertexInputLayout;
      SharedPtr<GpuBuffer> myVertexBuffer;
      SharedPtr<GpuBuffer> myIndexBuffer;
  };
//---------------------------------------------------------------------------//
}