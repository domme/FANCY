#ifndef INCLUDE_GPUBUFFERGL4_H
#define INCLUDE_GPUBUFFERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace FANCY { namespace Core { namespace Rendering { namespace GL4 {

  /// TODO: Make template?
  class GpuBufferGL4
  {
    public:
      GpuBufferGL4();
      ~GpuBufferGL4();

      bool isLocked() {return m_clStateInfos.isLocked;}
      bool isValid() {return m_uGLhandle != GLUINT_HANDLE_INVALID;}
      GLuint getGLhandle() {return m_uGLhandle;}
      uint getTotalSizeBytes() {return m_clParameters.uTotalSizeBytes; }

      void setBufferData(void* pData, uint uOffsetElements = 0, uint uNumElements = 0);
      void create(const BufferParameters& clParameters, void* pInitialData = nullptr);
      void destroy();
      void* lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0, uint uNumElements = 0);
      void unlock();
      
    private:
  //---------------------------------------------------------------------------//
      struct BufferParametersGL 
        : public BufferParameters
      {
        BufferParametersGL() : eInitialBufferTargetGL(0), eBindingQueryType(0),
          uAccessFlagsGL(0), uTotalSizeBytes(0) {}

        /// The OpenGL-binding point the buffer was initially created for
        GLenum eInitialBufferTargetGL;
        GLenum eBindingQueryType;
        GLuint uAccessFlagsGL;
        uint uTotalSizeBytes;
      };
  //---------------------------------------------------------------------------//
      struct BufferInfos {
        BufferInfos() : isLocked(0) {}

        uint isLocked : 1;
      };
  //---------------------------------------------------------------------------//
      BufferParametersGL m_clParameters;
      BufferInfos m_clStateInfos;
      GLuint m_uGLhandle;
  //---------------------------------------------------------------------------//
  };

} } } } // end of namespace FANCY::Core::Rendering::GL4


#endif  // INCLUDE_GPUBUFFERGL4_H