#include "GpuProgramCompilerGL4.h"
#include "GpuProgram.h"
#include "AdapterGL4.h"
#include "ShaderConstantsManager.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  namespace internal 
  {
    void getSizeFormatFromGLtype(GLenum typeGL, uint32& rSize, DataFormat& rFormat);
    VertexSemantics getVertexSemanticsFromName(const ObjectName& name);
    void getResourceTypeAndAccessTypeFromGLtype(GLenum typeGL, GpuResourceType& rType, 
                                                GpuResourceAccessType& rAccessType);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void internal::getSizeFormatFromGLtype(GLenum typeGL, uint32& rSize, DataFormat& rFormat)
  {
    rFormat = DataFormat::NONE;
    switch(typeGL)
    {
      case GL_FLOAT:       rSize = sizeof(float); rFormat = DataFormat::R_32F; break;     
      case GL_FLOAT_VEC2:  rSize = sizeof(glm::fvec2); rFormat = DataFormat::RG_32F; break;
      case GL_FLOAT_VEC3:  rSize = sizeof(glm::fvec3); rFormat = DataFormat::RGB_32F; break;
      case GL_FLOAT_VEC4:  rSize = sizeof(glm::fvec4); rFormat = DataFormat::RGBA_32F; break;
      case GL_DOUBLE:      break;
      case GL_DOUBLE_VEC2: break;
      case GL_DOUBLE_VEC3: break;
      case GL_DOUBLE_VEC4: break;
      case GL_INT:         break;
      case GL_INT_VEC2:    break;
      case GL_INT_VEC3:    break;
      case GL_INT_VEC4:    break;
      case GL_UNSIGNED_INT:      rSize = sizeof(uint32); rFormat = DataFormat::R_32UI; break;
      case GL_UNSIGNED_INT_VEC2: rSize = sizeof(glm::uvec2); rFormat = DataFormat::RG_32UI; break;
      case GL_UNSIGNED_INT_VEC3: rSize = sizeof(glm::uvec3); rFormat = DataFormat::RGB_32UI; break;
      case GL_UNSIGNED_INT_VEC4: rSize = sizeof(glm::uvec4); rFormat = DataFormat::RGBA_32UI; break;
      case GL_BOOL:      break;
      case GL_BOOL_VEC2: break;
      case GL_BOOL_VEC3: break;
      case GL_BOOL_VEC4: break;
      case GL_FLOAT_MAT2:   break;
      case GL_FLOAT_MAT3:   break;
      case GL_FLOAT_MAT4:   break;
      case GL_FLOAT_MAT2x3: break;
      case GL_FLOAT_MAT2x4: break;
      case GL_FLOAT_MAT3x2: break;
      case GL_FLOAT_MAT3x4: break;
      case GL_FLOAT_MAT4x2: break;
      case GL_FLOAT_MAT4x3: break;
      case GL_DOUBLE_MAT2:  break;
      case GL_DOUBLE_MAT3:  break;
      case GL_DOUBLE_MAT4:  break;
      case GL_DOUBLE_MAT2x3: break;
      case GL_DOUBLE_MAT2x4: break;
      case GL_DOUBLE_MAT3x2: break;
      case GL_DOUBLE_MAT3x4: break;
      case GL_DOUBLE_MAT4x2: break;
      case GL_DOUBLE_MAT4x3: break;
      case GL_SAMPLER_1D:                    break;
      case GL_SAMPLER_2D:                    break;
      case GL_SAMPLER_3D:                    break;
      case GL_SAMPLER_CUBE:                  break;
      case GL_SAMPLER_1D_SHADOW:             break;
      case GL_SAMPLER_2D_SHADOW:             break;
      case GL_SAMPLER_1D_ARRAY :             break;
      case GL_SAMPLER_2D_ARRAY :             break;
      case GL_SAMPLER_CUBE_MAP_ARRAY:        break;
      case GL_SAMPLER_1D_ARRAY_SHADOW:       break;
      case GL_SAMPLER_2D_ARRAY_SHADOW:       break;
      case GL_SAMPLER_2D_MULTISAMPLE:        break;
      case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:  break;
      case GL_SAMPLER_CUBE_SHADOW:           break;
      case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW: break;
      case GL_SAMPLER_BUFFER:                break;
      case GL_SAMPLER_2D_RECT:               break;
      case GL_SAMPLER_2D_RECT_SHADOW:        break;
      case GL_INT_SAMPLER_1D:                break;
      case GL_INT_SAMPLER_2D:                break;
      case GL_INT_SAMPLER_3D:                break;
      case GL_INT_SAMPLER_CUBE:              break;
      case GL_INT_SAMPLER_1D_ARRAY :         break;
      case GL_INT_SAMPLER_2D_ARRAY:          break;
      case GL_INT_SAMPLER_CUBE_MAP_ARRAY:    break;
      case GL_INT_SAMPLER_2D_MULTISAMPLE:    break;
      case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:          break;
      case GL_INT_SAMPLER_BUFFER:                         break;
      case GL_INT_SAMPLER_2D_RECT:                        break;
      case GL_UNSIGNED_INT_SAMPLER_1D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_2D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_3D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_CUBE:                  break;
      case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:              break;
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:              break;
      case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:        break;
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:        break;
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:  break;
      case GL_UNSIGNED_INT_SAMPLER_BUFFER:                break;
      case GL_UNSIGNED_INT_SAMPLER_2D_RECT:               break;
      case GL_IMAGE_1D:                                   break;
      case GL_IMAGE_2D:                                   break;
      case GL_IMAGE_3D:                                   break;
      case GL_IMAGE_2D_RECT:                              break;
      case GL_IMAGE_CUBE:                                 break;
      case GL_IMAGE_BUFFER:                               break;
      case GL_IMAGE_1D_ARRAY:                             break;
      case GL_IMAGE_2D_ARRAY:                             break;
      case GL_IMAGE_CUBE_MAP_ARRAY:                       break;
      case GL_IMAGE_2D_MULTISAMPLE:                       break;
      case GL_IMAGE_2D_MULTISAMPLE_ARRAY:                 break;
      case GL_INT_IMAGE_1D:                               break;
      case GL_INT_IMAGE_2D:                               break;
      case GL_INT_IMAGE_3D:                               break;
      case GL_INT_IMAGE_2D_RECT:                          break;
      case GL_INT_IMAGE_CUBE:                             break;
      case GL_INT_IMAGE_BUFFER:                           break;
      case GL_INT_IMAGE_1D_ARRAY:                         break;
      case GL_INT_IMAGE_2D_ARRAY:                         break;
      case GL_INT_IMAGE_CUBE_MAP_ARRAY:                   break;
      case GL_INT_IMAGE_2D_MULTISAMPLE:                   break;
      case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:             break;
      case GL_UNSIGNED_INT_IMAGE_1D:                      break;
      case GL_UNSIGNED_INT_IMAGE_2D:                      break;
      case GL_UNSIGNED_INT_IMAGE_3D:                      break;
      case GL_UNSIGNED_INT_IMAGE_2D_RECT:                 break;
      case GL_UNSIGNED_INT_IMAGE_CUBE:                    break;
      case GL_UNSIGNED_INT_IMAGE_BUFFER:                  break;
      case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:                break;
      case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:                break;
      case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:          break;
      case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:          break;
      case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:    break;
      case GL_UNSIGNED_INT_ATOMIC_COUNTER:                break;
    }

    ASSERT_M(rFormat != DataFormat::NONE, "Missing GL implementation");
  }
//---------------------------------------------------------------------------//
  void internal::getResourceTypeAndAccessTypeFromGLtype(GLenum typeGL, GpuResourceType& rType, 
                                                        GpuResourceAccessType& rAccessType)
  {
    rType = GpuResourceType::NONE;
    switch(typeGL)
    {
      case GL_SAMPLER_1D:         rType = GpuResourceType::TEXTURE_1D; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_2D:         rType = GpuResourceType::TEXTURE_2D; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_3D:         rType = GpuResourceType::TEXTURE_3D; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_CUBE:       rType = GpuResourceType::TEXTURE_CUBE; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_1D_SHADOW:  rType = GpuResourceType::TEXTURE_1D_SHADOW; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_2D_SHADOW:  rType = GpuResourceType::TEXTURE_2D_SHADOW; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      /*
      case GL_SAMPLER_1D_ARRAY :             break;
      case GL_SAMPLER_2D_ARRAY :             break;
      case GL_SAMPLER_CUBE_MAP_ARRAY:        break;
      case GL_SAMPLER_1D_ARRAY_SHADOW:       break;
      case GL_SAMPLER_2D_ARRAY_SHADOW:       break;
      case GL_SAMPLER_2D_MULTISAMPLE:        break;
      case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:  break;
      */
      case GL_SAMPLER_CUBE_SHADOW: rType = GpuResourceType::TEXTURE_CUBE_SHADOW; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW: break;
      case GL_SAMPLER_BUFFER:  rType = GpuResourceType::BUFFER_TEXTURE; rAccessType = GpuResourceAccessType::READ_ONLY; break;
      /*
      case GL_SAMPLER_2D_RECT:               break;
      case GL_SAMPLER_2D_RECT_SHADOW:        break;
      case GL_INT_SAMPLER_1D:                break;
      case GL_INT_SAMPLER_2D:                break;
      case GL_INT_SAMPLER_3D:                break;
      case GL_INT_SAMPLER_CUBE:              break;
      case GL_INT_SAMPLER_1D_ARRAY :         break;
      case GL_INT_SAMPLER_2D_ARRAY:          break;
      case GL_INT_SAMPLER_CUBE_MAP_ARRAY:    break;
      case GL_INT_SAMPLER_2D_MULTISAMPLE:    break;
      case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:          break;
      case GL_INT_SAMPLER_BUFFER:                         break;
      case GL_INT_SAMPLER_2D_RECT:                        break;
      case GL_UNSIGNED_INT_SAMPLER_1D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_2D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_3D:                    break;
      case GL_UNSIGNED_INT_SAMPLER_CUBE:                  break;
      case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:              break;
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:              break;
      case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:        break;
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:        break;
      case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:  break;
      case GL_UNSIGNED_INT_SAMPLER_BUFFER:                break;
      case GL_UNSIGNED_INT_SAMPLER_2D_RECT:               break;
      */
      case GL_IMAGE_1D: rType = GpuResourceType::TEXTURE_1D; rAccessType = GpuResourceAccessType::READ_WRITE; break;
      case GL_IMAGE_2D: rType = GpuResourceType::TEXTURE_2D; rAccessType = GpuResourceAccessType::READ_WRITE; break;
      case GL_IMAGE_3D: rType = GpuResourceType::TEXTURE_3D; rAccessType = GpuResourceAccessType::READ_WRITE; break;
      /*
      case GL_IMAGE_2D_RECT:                              break;
      case GL_IMAGE_CUBE:                                 break;
      */
      case GL_IMAGE_BUFFER: rType = GpuResourceType::BUFFER_TEXTURE; rAccessType = GpuResourceAccessType::READ_WRITE; break;
      /*
      case GL_IMAGE_1D_ARRAY:                             break;
      case GL_IMAGE_2D_ARRAY:                             break;
      case GL_IMAGE_CUBE_MAP_ARRAY:                       break;
      case GL_IMAGE_2D_MULTISAMPLE:                       break;
      case GL_IMAGE_2D_MULTISAMPLE_ARRAY:                 break;
      case GL_INT_IMAGE_1D:                               break;
      case GL_INT_IMAGE_2D:                               break;
      case GL_INT_IMAGE_3D:                               break;
      case GL_INT_IMAGE_2D_RECT:                          break;
      case GL_INT_IMAGE_CUBE:                             break;
      case GL_INT_IMAGE_BUFFER:                           break;
      case GL_INT_IMAGE_1D_ARRAY:                         break;
      case GL_INT_IMAGE_2D_ARRAY:                         break;
      case GL_INT_IMAGE_CUBE_MAP_ARRAY:                   break;
      case GL_INT_IMAGE_2D_MULTISAMPLE:                   break;
      case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:             break;
      case GL_UNSIGNED_INT_IMAGE_1D:                      break;
      case GL_UNSIGNED_INT_IMAGE_2D:                      break;
      case GL_UNSIGNED_INT_IMAGE_3D:                      break;
      case GL_UNSIGNED_INT_IMAGE_2D_RECT:                 break;
      case GL_UNSIGNED_INT_IMAGE_CUBE:                    break;
      case GL_UNSIGNED_INT_IMAGE_BUFFER:                  break;
      case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:                break;
      case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:                break;
      case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:          break;
      case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:          break;
      case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:    break;
      case GL_UNSIGNED_INT_ATOMIC_COUNTER:                break;
      */
    }

    ASSERT_M(rType != GpuResourceType::NONE, "Missing GL implementation");
  }
//---------------------------------------------------------------------------//
  VertexSemantics internal::getVertexSemanticsFromName(const ObjectName& name)
  {
    if (name == _N(v_position)) {
      return VertexSemantics::POSITION;
    } else if (name == _N(v_normal)) {
      return VertexSemantics::NORMAL;
    } else if (name == _N(v_tangent)) {
      return VertexSemantics::TANGENT;
    } else if (name == _N(v_bitangent)) {
      return VertexSemantics::BITANGENT;
    } else if (name == _N(v_texcoord) || (name == _N(v_texcoord0))) {
      return VertexSemantics::TEXCOORD0;
    } else if (name == _N(v_texcoord1)) {
      return VertexSemantics::TEXCOORD1;
    } else if (name == _N(v_texcoord2)) {
      return VertexSemantics::TEXCOORD2;
    } else if (name == _N(v_texcoord3)) {
      return VertexSemantics::TEXCOORD3;
    } else if (name == _N(v_texcoord4)) {
      return VertexSemantics::TEXCOORD4;
    } else if (name == _N(v_texcoord5)) {
      return VertexSemantics::TEXCOORD5;
    } else if (name == _N(v_texcoord6)) {
      return VertexSemantics::TEXCOORD6;
    } else if (name == _N(v_texcoord7)) {
      return VertexSemantics::TEXCOORD7;
    }

    ASSERT_M(false, "Unknown vertex input detected");
    return VertexSemantics::POSITION;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GpuProgramCompilerGL4::GpuProgramCompilerGL4()
  {

  }
//---------------------------------------------------------------------------//
  GpuProgramCompilerGL4::~GpuProgramCompilerGL4()
  {

  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerGL4::compileFromSource( const String& szSource, 
    const ShaderStage& eShaderStage, const ObjectName& name, GpuProgramGL4& rGpuProgram )
  {
    ASSERT_M(!szSource.empty(), "Invalid shader source");

    GLenum eShaderStageGL = Adapter::toGLType(eShaderStage);
    
    const char* szShaderSource_cstr = szSource.c_str();
    GLuint uProgramHandle = glCreateShaderProgramv(eShaderStageGL, 1, &szShaderSource_cstr);

    int iLogLengthChars = 0;
    glGetProgramiv(uProgramHandle, GL_INFO_LOG_LENGTH, &iLogLengthChars);
    ASSERT(iLogLengthChars < kMaxNumLogChars);

    if (iLogLengthChars > 0)
    {
      glGetProgramInfoLog(uProgramHandle, kMaxNumLogChars, &iLogLengthChars, m_LogBuffer);
      log_Info(m_LogBuffer);
    }
    // clear the log again
    memset(m_LogBuffer, 0x0, sizeof(m_LogBuffer));

    int iProgramLinkStatus = GL_FALSE;
    glGetProgramiv(uProgramHandle, GL_LINK_STATUS, &iProgramLinkStatus);

    bool success = iProgramLinkStatus;
    if (success)
    {
      rGpuProgram.m_eShaderStage = eShaderStage;
      rGpuProgram.m_uProgramHandleGL = uProgramHandle;
      rGpuProgram.m_Name = name;

      success = reflectProgram(rGpuProgram);
    }

    if (!success) 
    {
      log_Error(String("GpuProgram ") + name + " failed to compile" );
      glDeleteProgram(uProgramHandle);
    }
    
    return success;
  }
//---------------------------------------------------------------------------//
  bool GpuProgramCompilerGL4::reflectProgram( GpuProgramGL4& rGpuProgram )
  {
    GLuint uProgramHandle = rGpuProgram.m_uProgramHandleGL;
    ASSERT(uProgramHandle != GLUINT_HANDLE_INVALID);

    if (rGpuProgram.getShaderStage() == ShaderStage::VERTEX)
    {
      reflectVertexInputs(uProgramHandle, rGpuProgram.m_clVertexInputLayout);
    }
    else if(rGpuProgram.getShaderStage() == ShaderStage::FRAGMENT)
    {
      reflectFragmentOutputs(uProgramHandle, rGpuProgram.m_vFragmentOutputs);
    }

    reflectConstants(uProgramHandle);

    reflectResources(uProgramHandle, rGpuProgram.m_vReadTextureInfos,
      rGpuProgram.m_vReadBufferInfos, rGpuProgram.m_vWriteTextureInfos, 
      rGpuProgram.m_vWriteBufferInfos);

    return true;
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectResources( GLuint uProgram, 
    GpuResourceInfoList& rReadTextureInfos, GpuResourceInfoList& rReadBufferInfos, 
    GpuResourceInfoList& rWriteTextureInfos, GpuResourceInfoList& rWriteBufferInfos) const
  {
    // We assume that all uniforms which don't  belong to a block are resources (i.e. buffers, samplers, textures)

    GLint numResources = 0;  // Will contain the overall number of uniforms - including block uniforms
    glGetProgramInterfaceiv(uProgram, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numResources);
    const GLenum vProperties[] = {GL_BLOCK_INDEX, GL_TYPE​, GL_NAME_LENGTH​, GL_LOCATION};

    for(int iUniform = 0; iUniform < numResources; ++iUniform)
    {
      GLint vPropertyValues[_countof(vProperties)];
      glGetProgramResourceiv(uProgram, GL_UNIFORM, iUniform, _countof(vProperties), 
         vProperties, _countof(vProperties), nullptr, vPropertyValues);

      // If the uniform belongs to a block: Skip it!
      if(vPropertyValues[0] != -1)
      {
        continue;
      }
      
      String szName(vPropertyValues[GL_NAME_LENGTH], ' ');
      glGetProgramResourceName(uProgram, GL_UNIFORM, iUniform, szName.size(), nullptr, &szName[0]);

      // Construct the resourceInfo object
      GpuProgramResourceInfo resourceInfo;
      resourceInfo.name = szName;
      resourceInfo.u32RegisterIndex = vPropertyValues[3];
      internal::getResourceTypeAndAccessTypeFromGLtype(vProperties[1], 
        resourceInfo.eResourceType, resourceInfo.eAccessType);
      
      resourceInfo.bindingTargetGL = 
        Adapter::mapResourceTypeToGLbindingTarget(resourceInfo.eResourceType);

      // The "format qualifier" for glsl images. TODO: can this be reflected?
      // resourceInfo.dataFormatGL =
      
      const bool isTexture = resourceInfo.eResourceType != GpuResourceType::NONE
                    && resourceInfo.eResourceType >= GpuResourceType::TEXTURE_1D 
                    && resourceInfo.eResourceType <= GpuResourceType::BUFFER_TEXTURE;

      const bool isBuffer = resourceInfo.eResourceType == GpuResourceType::BUFFER;

      const bool isReadOnly = resourceInfo.eAccessType == GpuResourceAccessType::READ_ONLY;

      if (isTexture)
      {
        if (isReadOnly)
          rReadTextureInfos.push_back(resourceInfo);
        else 
          rWriteTextureInfos.push_back(resourceInfo);
      }
      else if (isBuffer)
      {
        if (isReadOnly)
          rReadBufferInfos.push_back(resourceInfo);
        else
          rWriteBufferInfos.push_back(resourceInfo);
      }
      else
      {
        // Non-implemented case
        ASSERT_M(false, "GpuResource type not supported yet");
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectVertexInputs(GLuint uProgram, VertexInputLayout& rVertexLayout) const
  {
    rVertexLayout.clear();

    GLint iNumResources = 0;
    const GLenum eInterface = GL_PROGRAM_INPUT;
    glGetProgramInterfaceiv(uProgram, eInterface, GL_ACTIVE_RESOURCES, &iNumResources);

    const GLenum vProperties[] = {GL_TYPE, GL_LOCATION};
    for (uint32 i = 0u; i < iNumResources; ++i)
    {
      GLchar _name[128] = {0u};
      glGetProgramResourceName(uProgram, eInterface, i, _countof(_name), nullptr, _name);

      GLint vPropertyValues[_countof(vProperties)] = {0x0};

      glGetProgramResourceiv(uProgram, eInterface, i,
        _countof(vProperties), vProperties, _countof(vPropertyValues), 
        nullptr, vPropertyValues );

      GLuint _type        = vPropertyValues[0];
      GLuint _location    = vPropertyValues[1];

      // Convert to engine-types
      DataFormat format(DataFormat::NONE);
      uint32 u32SizeBytes = 0u;
      internal::getSizeFormatFromGLtype(_type, u32SizeBytes, format);

      ObjectName name = _name;
      VertexSemantics semantics = internal::getVertexSemanticsFromName(name);
      
      VertexInputElement vertexElement;
      vertexElement.name = name;
      vertexElement.eSemantics = semantics;
      vertexElement.u32RegisterIndex = _location;
      vertexElement.u32SizeBytes = u32SizeBytes;
      vertexElement.eFormat = format;
      
      rVertexLayout.addVertexInputElement(vertexElement);
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectFragmentOutputs(GLuint uProgram, ShaderStageFragmentOutputList& vFragmentOutputs) const
  {
    vFragmentOutputs.clear();

    GLint iNumResources = 0u;
    const GLenum eInterface = GL_PROGRAM_OUTPUT;
    glGetProgramInterfaceiv(uProgram, eInterface, GL_ACTIVE_RESOURCES, &iNumResources);

    const GLenum vProperties[] = {GL_TYPE, GL_LOCATION};
    for (uint32 i = 0u; i < iNumResources; ++i)
    {
      GLchar _name[128] = {0u};
      glGetProgramResourceName(uProgram, eInterface, i, _countof(_name), nullptr, _name);

      GLint vPropertyValues[_countof(vProperties)] = {0x0};

      glGetProgramResourceiv(uProgram, eInterface, i,
        _countof(vProperties), vProperties, _countof(vPropertyValues), 
        nullptr, vPropertyValues );

      GLuint _type        = vPropertyValues[0];
      GLuint _location    = vPropertyValues[1];

      DataFormat format(DataFormat::NONE);
      uint32 _u32Size = 0;
      internal::getSizeFormatFromGLtype(_type, _u32Size, format);

      ShaderStageFragmentOutput output;
      output.name = _name;
      output.uRtIndex = _location;
      output.eFormat = format;

      vFragmentOutputs.push_back(output);
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectConstants( GLuint uProgram )
  {
    GLint iNumUniformBlocks = 0u;
    const GLenum eInterface = GL_UNIFORM_BLOCK;
    glGetProgramInterfaceiv(uProgram, eInterface, GL_ACTIVE_RESOURCES, &iNumUniformBlocks);
    
    for (uint32 iBlock = 0u; iBlock < iNumUniformBlocks; ++iBlock)
    {
      GLchar _name[128] = {0u};
      glGetProgramResourceName(uProgram, eInterface, iBlock, _countof(_name), nullptr, _name);
      ObjectName blockName = _name;
      ConstantBufferType eCbufferType = 
        ShaderConstantsManager::getInstance().getConstantBufferTypeFromName(blockName);
      ASSERT_M(eCbufferType != ConstantBufferType::NONE, "Invalid constant buffer name");

      //uint32 uBlockIndex = glGetProgramResourceIndex(uProgram, eInterface, _name);

      const GLenum vProperties[] = {GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES};
      GLint vPropertyValues[_countof(vProperties)] = {0x0};
      glGetProgramResourceiv(uProgram, eInterface, iBlock, _countof(vProperties), 
        vProperties, _countof(vProperties), nullptr, vPropertyValues);

      const uint32 uUniformBlockBinding = vPropertyValues[0];
      const uint32 uNumUniformsInBlock = vPropertyValues[1];
      
      // Sanity-check of the binding point. We require it to match the eCbufferType
      ASSERT_M(uUniformBlockBinding < (uint) ConstantBufferType::NUM &&
               (ConstantBufferType) uUniformBlockBinding == eCbufferType, 
               "CBuffer-name does not match its expected binding point");

      // Acquire the indices of all active uniforms in the block
      FixedArray<GLint, kMaxNumConstantBufferElements> vUniformIndices;
      vUniformIndices.resize(uNumUniformsInBlock);

      const GLenum propActiveUniformIndices = GL_ACTIVE_VARIABLES;
      glGetProgramResourceiv(uProgram, eInterface, iBlock, 1, &propActiveUniformIndices, 
        vUniformIndices.size(), nullptr, &vUniformIndices[0]);

      for (uint32 iUniform = 0u; 
        iUniform < uNumUniformsInBlock; ++iUniform )
      {
        const uint32 uUniformIndex = vUniformIndices[iUniform];

        const GLenum vProperties[] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET};
        GLint vPropertyValues[_countof(vProperties)] = {0x0};
        
        glGetProgramResourceiv(uProgram, GL_UNIFORM, uUniformIndex, _countof(vProperties), 
          vProperties, _countof(vProperties), nullptr, vPropertyValues);

        GLchar _name[128] = {0u};
        ASSERT(vPropertyValues[0] <= _countof(_name));
        glGetProgramResourceName(uProgram, GL_UNIFORM, uUniformIndex, 
          _countof(_name), nullptr, _name);

        uint32 uSizeBytes;
        DataFormat eFormat;
        internal::getSizeFormatFromGLtype(vPropertyValues[1], uSizeBytes, eFormat);
        
        ConstantBufferElement cBufferElement;
        cBufferElement.name = _name;
        cBufferElement.uOffsetBytes = vPropertyValues[2];
        cBufferElement.eFormat = eFormat;
        cBufferElement.uSizeBytes = uSizeBytes;
        
        ConstantSemantics eSemantics = 
          ShaderConstantsManager::getInstance().getSemanticFromName(cBufferElement.name);

        ShaderConstantsManager::getInstance().registerElement(cBufferElement, eSemantics, eCbufferType);
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectStageInputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList) const
  {
    // TODO: Implement
  }
//---------------------------------------------------------------------------//
  void GpuProgramCompilerGL4::reflectStageOutputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList) const
  {
    // TODO: Implement
  }



  //---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
} } } } // end of namespace Fancy::Core::Rendering:GL4