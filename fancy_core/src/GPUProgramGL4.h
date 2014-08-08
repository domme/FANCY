#ifndef INCLUDE_GPUPROGRAMGL4_H
#define INCLUDE_GPUPROGRAMGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramResource.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  class GpuProgramGL4
  {
  //---------------------------------------------------------------------------//
    friend class GpuProgramCompilerGL4;
  //---------------------------------------------------------------------------//
    struct InterfaceElement
    {
      ObjectName name;
      GLuint uLocation;
      GLuint uArraySize;
      GLuint uOffset;
      GLenum eTypeGL;
      GLuint uAtomicCountBufIndex;

    };
  //---------------------------------------------------------------------------//
    public:
      GpuProgramGL4();
      ~GpuProgramGL4();
    //---------------------------------------------------------------------------//
      const ObjectName& getName() const {return m_Name;}
      GLuint getProgramHandle() const {return m_uProgramHandleGL;}
      ShaderStage getShaderStage() const {return m_eShaderStage;}
      const GpuResourceInfoList& getResourceInfoList() const {return m_vResourceInfos;}
    //---------------------------------------------------------------------------//
    protected:
      void destroy();

      ObjectName m_Name;
      GLuint m_uProgramHandleGL;
      ShaderStage m_eShaderStage;
      GpuResourceInfoList m_vResourceInfos;
    //---------------------------------------------------------------------------//
  };
//---------------------------------------------------------------------------//
} } } }  // end of namespace Fancy::Core::Rendering::GL4


#endif  // INCLUDE_GPUPROGRAMGL4_H