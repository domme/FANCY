#ifndef INCLUDE_GPUPROGRAMGL4_H
#define INCLUDE_GPUPROGRAMGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "GpuProgramResource.h"
#include "VertexInputLayout.h"
#include "Serializable.h"
#include "GpuProgramDesc.h"

namespace Fancy{namespace IO{
  class ObjectFactory;
  class Serializer;
}}

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  struct ShaderStageInterfaceElement
  {
    SERIALIZABLE(ShaderStageInterfaceElement)
    static ObjectName getTypeName() { return _N(ShaderStageInterfaceElement); }
    const ObjectName& getName() { return ObjectName::blank; }
    void serialize(IO::Serializer* aSerializer);

    ShaderStageInterfaceElement() : 
      uLocation(0u), uArraySize(0u), uArrayStride(0u), uOffset(0u), 
      uBlockIndex(0u), eTypeGL(0u), uAtomicCountBufIndex(0u) {}

    ObjectName name;
    GLuint uLocation;
    GLuint uArraySize;
    GLuint uArrayStride;
    GLuint uOffset;
    GLuint uBlockIndex;
    GLenum eTypeGL;
    GLuint uAtomicCountBufIndex;
  };
//---------------------------------------------------------------------------//
  struct ShaderStageFragmentOutput
  {
    SERIALIZABLE(ShaderStageFragmentOutput)
      static ObjectName getTypeName() { return _N(ShaderStageFragmentOutput); }
    const ObjectName& getName() { return ObjectName::blank; }
    void serialize(IO::Serializer* aSerializer);

    ShaderStageFragmentOutput() :
      uRtIndex(0u), uLocation(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    /// Name in glsl
    ObjectName name;
    /// Rendertarget index
    GLuint uRtIndex;
    /// Register index
    GLuint uLocation;
    /// expected format of the output
    DataFormat eFormat;
     // Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
    uint8 uFormatComponentCount; 
  };
//---------------------------------------------------------------------------//
  const uint32 kMaxNumShaderStageInterfaceElements = 16;
  typedef FixedArray<ShaderStageInterfaceElement, kMaxNumShaderStageInterfaceElements> ShaderStageInterfaceList;
  typedef FixedArray<ShaderStageFragmentOutput, kMaxNumShaderStageInterfaceElements> ShaderStageFragmentOutputList;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  /// The information provided by an (extenral) GpuProgramCompiler
  struct GpuProgramCompilerOutputGL4
  {
    GpuProgramCompilerOutputGL4();
    ObjectName name;
    GLuint uProgramHandleGL;
    ShaderStage eShaderStage;
    ShaderVertexInputLayout clVertexInputLayout;
    ShaderStageInterfaceList vInputInterfaces;
    ShaderStageInterfaceList vOutputInterfaces;
    ShaderStageFragmentOutputList vFragmentOutputs;
    GpuResourceInfoList vReadTextureInfos;
    GpuResourceInfoList vReadBufferInfos;
    GpuResourceInfoList vWriteTextureInfos;
    GpuResourceInfoList vWriteBufferInfos;
    ConstantBufferElementList myConstantBufferElements;
    String myShaderCode;
    String myShaderFilename;  /// Platform-independent shader filename (e.g. "MaterialForward")
    GpuProgramPermutation myPermutation;
  };
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class GpuProgramGL4
  {
  //---------------------------------------------------------------------------//
    friend class GpuProgramCompilerGL4;
    friend class RendererGL4;
    friend class IO::ObjectFactory;
  //---------------------------------------------------------------------------//
    public:
      GpuProgramGL4();
      ~GpuProgramGL4();
    //---------------------------------------------------------------------------//
      void SetFromCompilerOutput(const GpuProgramCompilerOutputGL4& _desc);
    //---------------------------------------------------------------------------//
      const ObjectName& getName() const {return m_Name;}
      static ObjectName getTypeName() { return _N(GpuProgram); }
      void serialize(IO::Serializer* aSerializer);

      bool operator==(const GpuProgramDesc& anOtherDesc) const;
      GpuProgramDesc GetDescription() const;

      GLuint getProgramHandle() const {return m_uProgramHandleGL;}
      ShaderStage getShaderStage() const {return m_eShaderStage;}
      const GpuResourceInfoList& getReadTextureInfoList() const {return m_vReadTextureInfos;}
      const GpuResourceInfoList& getReadBufferInfoList() const {return m_vReadBufferInfos;}
      const GpuResourceInfoList& getWriteTextureInfoList() const {return m_vWriteTextureInfos;}
      const GpuResourceInfoList& getWriteBufferInfoList() const {return m_vWriteBufferInfos;}
      const ShaderVertexInputLayout* getVertexInputLayout() const {return &m_clVertexInputLayout;}
    //---------------------------------------------------------------------------//
    protected:
      void destroy();
      
      /// Name of the shader as loaded from disk
      ObjectName m_Name;
      /// GL-internal handle to the program object
      GLuint m_uProgramHandleGL;
      /// ShaderStage this program defines
      ShaderStage m_eShaderStage;
      /// Expected layout of incoming vertices. Only valid for vertex-programs
      ShaderVertexInputLayout m_clVertexInputLayout;
      /// List of incoming inter-stage varyings. Not valid for vertex-programs
      ShaderStageInterfaceList m_vInputInterfaces;
      /// List of outgoing inter-stage varyings.
      ShaderStageInterfaceList m_vOutputInterfaces;
      /// List of output-elements in the fragment shader stage
      ShaderStageFragmentOutputList m_vFragmentOutputs;

      String myShaderCode;  // TODO: Only temporary until shader binaries have been implemented
      String myShaderFilename;  /// Platform-independent shader filename (e.g. "MaterialForward")
      GpuProgramPermutation myPermutation;
      
      /// Lists of resources defined in the shader (buffers, textures, ...)
      GpuResourceInfoList m_vReadTextureInfos;
      GpuResourceInfoList m_vReadBufferInfos;
      GpuResourceInfoList m_vWriteTextureInfos;
      GpuResourceInfoList m_vWriteBufferInfos;
      ConstantBufferElementList myConstantBufferElements;
      // TODO: Add lists for other opaque types (e.g. atomics)
    //---------------------------------------------------------------------------//
  };
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4

#endif

#endif  // INCLUDE_GPUPROGRAMGL4_H