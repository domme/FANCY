#pragma once

#if defined (RENDERER_DX12)
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"

namespace Fancy{ namespace IO{
class ObjectFactory;
}}

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

      class GpuProgramDX12
      {
        friend class GpuProgramCompilerDX12;
        friend class RendererDX12;
        friend class IO::ObjectFactory;

      public:
        GpuProgramDX12();
        ~GpuProgramDX12();
        //---------------------------------------------------------------------------//
        const ObjectName& getName() const { return myName; }
        static ObjectName getTypeName() { return _N(GpuProgram); }
        void serialize(IO::Serializer* aSerializer);
        
        ShaderStage getShaderStage() const { return myStage; }
        const GpuResourceInfoList& getReadTextureInfoList() const { return myReadTextureInfos; }
        const GpuResourceInfoList& getReadBufferInfoList() const { return myReadBufferInfos; }
        const GpuResourceInfoList& getWriteTextureInfoList() const { return myWriteTextureInfos; }
        const GpuResourceInfoList& getWriteBufferInfoList() const { return myWriteBufferInfos; }
        const VertexInputLayout* getVertexInputLayout() const { return &myInputLayout; }

      private:
        ObjectName myName;
        ShaderStage myStage;

        GpuResourceInfoList myReadTextureInfos;
        GpuResourceInfoList myReadBufferInfos;
        GpuResourceInfoList myWriteTextureInfos;
        GpuResourceInfoList myWriteBufferInfos;
        VertexInputLayout myInputLayout;
      };

    }
  }
}

#endif


