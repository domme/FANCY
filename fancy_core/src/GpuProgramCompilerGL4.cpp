#include "GpuProgramCompilerGL4.h"
#include "GpuProgram.h"
#include "ShaderConstantsManager.h"
#include "FileReader.h"
#include "StringUtil.h"
#include "MathUtil.h"

#include <deque>

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  namespace Internal 
  {
    static const uint32 kMaxNumLogChars = 10000u;

    void getSizeFormatFromGLtype(GLenum typeGL, uint32& rSize, DataFormat& rFormat, uint32& rNumComponents);
    VertexSemantics getVertexSemanticsFromName(const ObjectName& name);
    void getResourceTypeAndAccessTypeFromGLtype(GLenum typeGL, GpuResourceType& rType, GpuResourceAccessType& rAccessType);
    String getDefineFromShaderStage(ShaderStage _eStage);
  }
//---------------------------------------------------------------------------//
  namespace Preprocess
  {
    struct FileBlock {
      uint32 startLine;
      uint32 endLine;
      uint nameHash;
      String fileName;
    };

    struct ShaderSourceInfo {
      uint32 numLines;
      FixedArray<FileBlock, 16u> vFileBlocks;
    };

    uint32 getIteratorPosition(std::list<String>::const_iterator& _it, std::list<String>& _lineList);
    void preprocessShaderSource(const String& shaderFilename, std::list<String>& sourceLines, const ShaderStage& eShaderStage, ShaderSourceInfo& info);
    FileBlock* getFileBlockForLine(uint32 _lineNumber, ShaderSourceInfo& _info);
    const FileBlock* getFileBlockForLine(uint32 _lineNumber, const ShaderSourceInfo& _info);
    uint32 getLocalLine(uint32 _globalLine, const ShaderSourceInfo& _info);
    void insertNewFileBlock(FileBlock* _pCurrBlock, uint32 _currLine, uint32 _numLinesNewBlock, const String& _newBlockName, ShaderSourceInfo& _info);
  }
//---------------------------------------------------------------------------//
  namespace Compile
  {
    bool compileFromSource(const String& szSource, const Preprocess::ShaderSourceInfo& sourceInfo, const ShaderStage& eShaderStage, GpuProgramDescriptionGL4& _rDesc);
    void outputCompilerLogMsg(const char* _shaderCompilerLogMsg, const Preprocess::ShaderSourceInfo& _sourceInfo);
    bool reflectProgram(GpuProgramDescriptionGL4& _rDesc);
    void reflectConstants(GLuint uProgram);
    void reflectResources( GLuint uProgram, GpuResourceInfoList& rReadTextureInfos, GpuResourceInfoList& rReadBufferInfos, GpuResourceInfoList& rWriteTextureInfos, GpuResourceInfoList& rWriteBufferInfos);
    void reflectVertexInputs(GLuint uProgram, VertexInputLayout& rVertexLayout);
    void reflectFragmentOutputs(GLuint uProgram, ShaderStageFragmentOutputList& vFragmentOutputs);
    void reflectStageInputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList);
    void reflectStageOutputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void Internal::getSizeFormatFromGLtype(GLenum typeGL, uint32& rSize, DataFormat& rFormat, uint32& rNumComponents)
  {
    rFormat = DataFormat::NONE;
    rNumComponents = 1u;
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
      case GL_FLOAT_MAT2:   rSize = sizeof(glm::mat2); rFormat = DataFormat::RG_32F; rNumComponents = 2u; break;
      case GL_FLOAT_MAT3:   rSize = sizeof(glm::mat3); rFormat = DataFormat::RGB_32F; rNumComponents = 3u; break;
      case GL_FLOAT_MAT4:   rSize = sizeof(glm::mat4); rFormat = DataFormat::RGBA_32F; rNumComponents = 4u; break;
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
  void Internal::getResourceTypeAndAccessTypeFromGLtype(GLenum typeGL, GpuResourceType& rType, 
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
  uint32 Preprocess::getIteratorPosition(std::list<String>::const_iterator& _it, std::list<String>& _lineList)
  {
    uint32 pos = 1u;
    std::list<String>::iterator it = _lineList.begin();
    while (it != _it && it != _lineList.end())
    {
      ++it; ++pos;
    }

    return pos;
  }
//---------------------------------------------------------------------------//
  VertexSemantics Internal::getVertexSemanticsFromName(const ObjectName& name)
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
  String Internal::getDefineFromShaderStage(ShaderStage _eStage)
  {
    switch (_eStage)
    {
    case Fancy::Rendering::ShaderStage::VERTEX:
      return "PROGRAM_TYPE_VERTEX";
    case Fancy::Rendering::ShaderStage::FRAGMENT:
      return "PROGRAM_TYPE_FRAGMENT";
    case Fancy::Rendering::ShaderStage::GEOMETRY:
      return "PROGRAM_TYPE_GEOMETRY";
    case Fancy::Rendering::ShaderStage::TESS_HULL:
      return "PROGRAM_TYPE_TESS_HULL";
    case Fancy::Rendering::ShaderStage::TESS_DOMAIN:
      return "PROGRAM_TYPE_TESS_DOMAIN";
    case Fancy::Rendering::ShaderStage::COMPUTE:
      return "PROGRAM_TYPE_COMPUTE";
    default:
      return "";
    }
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  Preprocess::FileBlock* Preprocess::getFileBlockForLine(uint32 _lineNumber, ShaderSourceInfo& _info)
  {
    for (uint32 i = 0u; i < _info.vFileBlocks.size(); ++i)
    {
      FileBlock* block = &_info.vFileBlocks[i];
      if (_lineNumber >= block->startLine && _lineNumber <= block->endLine)
      {
        return block;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  const Preprocess::FileBlock* Preprocess::getFileBlockForLine(uint32 _lineNumber, const ShaderSourceInfo& _info)
  {
    for (uint32 i = 0u; i < _info.vFileBlocks.size(); ++i)
    {
      const FileBlock* block = &_info.vFileBlocks[i];
      if (_lineNumber >= block->startLine && _lineNumber <= block->endLine)
      {
        return block;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  uint32 Preprocess::getLocalLine(uint32 _globalLine, const ShaderSourceInfo& _info)
  {
    const FileBlock* pBlock = getFileBlockForLine(_globalLine, _info);
    if (!pBlock)
    {
      return _globalLine;
    }

    uint32 numLinesFromOtherFiles = 0u;
    for (uint32 i = 0u; i < _info.vFileBlocks.size(); ++i)
    {
      const FileBlock& block = _info.vFileBlocks[i];
      if (block.nameHash != pBlock->nameHash && block.endLine < _globalLine)
      {
        numLinesFromOtherFiles += (block.endLine - block.startLine) + 1u;
      }
    }

    return _globalLine - numLinesFromOtherFiles;
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void Preprocess::insertNewFileBlock(FileBlock* _pCurrBlock, uint32 _currLine, uint32 _numLinesNewBlock, const String& _newBlockName, ShaderSourceInfo& _info)
  {
    // Construct a block after the include from the current block
    _info.vFileBlocks.push_back(*_pCurrBlock);
    FileBlock* pBlockAfterNewBlock = &_info.vFileBlocks.back();
    pBlockAfterNewBlock->startLine = _currLine + 1u;
    pBlockAfterNewBlock->endLine = _pCurrBlock->endLine;
    
    // close the current block
    _pCurrBlock->endLine = _currLine;

    // Advance all following blocks by the number of new lines
    for (uint32 i = 0u; i < _info.vFileBlocks.size(); ++i)
    {
      FileBlock& rBlock = _info.vFileBlocks[i];
      if (rBlock.startLine < _currLine && rBlock.endLine > _currLine)
      {
        rBlock.endLine += _numLinesNewBlock;
      }

      else if (rBlock.startLine > _currLine)
      {
        rBlock.startLine += _numLinesNewBlock;
        rBlock.endLine += _numLinesNewBlock;
      }
    }

    // Construct the new block
    _info.vFileBlocks.push_back(FileBlock());
    FileBlock* pNewBlock = &_info.vFileBlocks.back();
    pNewBlock->startLine = _currLine + 1u;
    pNewBlock->endLine = _currLine + _numLinesNewBlock;
    pNewBlock->fileName = _newBlockName;
    pNewBlock->nameHash = MathUtil::hashFromString(pNewBlock->fileName);
  }
//---------------------------------------------------------------------------//
  void Preprocess::preprocessShaderSource(const String& shaderFilename, std::list<String>& sourceLines, const ShaderStage& eShaderStage, ShaderSourceInfo& info)
  {
    const uint32 kMaxNumDefines = 64u;
    FixedArray<String, kMaxNumDefines> vDefines;
    vDefines.push_back("#define " + Internal::getDefineFromShaderStage(eShaderStage));
    // ... more to come?

    const String kIncludeSearchKey = "#include \"";
    const String kCommentStartBlock = "/*";
    const String kCommentEndBlock = "*/";
    const String kLineComment = "//";
    const String kVersion ="#version";

    bool definesInserted = false;
    bool inCommentBlock = false;
    uint32 lineNumber = 1u;
    info.vFileBlocks.push_back(FileBlock());
    FileBlock* pCurrentSourceBlock = &info.vFileBlocks[0];
    pCurrentSourceBlock->startLine = lineNumber;
    pCurrentSourceBlock->endLine = sourceLines.size();
    pCurrentSourceBlock->fileName = shaderFilename;
    pCurrentSourceBlock->nameHash = MathUtil::hashFromString(pCurrentSourceBlock->fileName);
    for (std::list<String>::iterator itLine = sourceLines.begin(); itLine != sourceLines.end(); ++itLine, ++lineNumber)
    {
      const String& line = *itLine;
      if (line.empty()) continue;

      pCurrentSourceBlock = getFileBlockForLine(lineNumber, info);
      
      const size_t posCommentStartBlock = line.find(kCommentStartBlock);
      const bool hasCommentStartBlock = posCommentStartBlock != std::string::npos;
      const size_t posCommentEndBlock = line.find(kCommentEndBlock);
      const bool hasCommentEndBlock = posCommentEndBlock != std::string::npos;
      const size_t posLineComment = line.find(kLineComment);
      const bool hasLineComment = posLineComment != std::string::npos;

      if (!inCommentBlock && hasCommentStartBlock && (!hasLineComment || posLineComment > posCommentStartBlock))
      {
        inCommentBlock = true;
      }

      if (inCommentBlock && hasCommentEndBlock && (!hasCommentStartBlock || posCommentStartBlock < posCommentEndBlock))
      {
        inCommentBlock = false;
      }

      if (inCommentBlock || (hasLineComment && posLineComment == 0u))
      {
        continue;
      }

      if (!definesInserted)
      {
        size_t posVersion = line.find(kVersion);
        if (posVersion != std::string::npos)
        {
          insertNewFileBlock(pCurrentSourceBlock, lineNumber, vDefines.size(), 
            pCurrentSourceBlock->fileName + " - defines", info);

          ++itLine;
          for (uint32 i = 0u; i < vDefines.size(); ++i)
          {
            sourceLines.insert(itLine, vDefines[i]);
          }

          definesInserted = true;
          lineNumber = getIteratorPosition(itLine, sourceLines);
        }
      }

      size_t posInclude = line.find(kIncludeSearchKey);
      bool hasInclude = posInclude != std::string::npos;
     
      if (hasInclude)
      {
        size_t posAfterInclude = posInclude + kIncludeSearchKey.length();
        size_t posIncludeEnd = line.find("\"", posAfterInclude);
        String includeFileName = line.substr(posAfterInclude, posIncludeEnd - posAfterInclude);
        std::list<String> includeFileLines;
        IO::FileReader::ReadTextFileLines(includeFileName, includeFileLines);

        if (!includeFileLines.empty())
        {
          insertNewFileBlock(pCurrentSourceBlock, lineNumber,
            includeFileLines.size(), includeFileName, info);
          
          // Comment out the include line
          itLine->insert(0u, "// "); 

          std::list<String>::iterator itBeforeInsert = itLine;
          sourceLines.insert(++itLine, includeFileLines.begin(), includeFileLines.end());
          itLine = itBeforeInsert;
          lineNumber = getIteratorPosition(itLine, sourceLines);
          continue;
        }
      }
    }
  }
//---------------------------------------------------------------------------//  
//---------------------------------------------------------------------------//
  void Compile::outputCompilerLogMsg(const char* _shaderCompilerLogMsg, const Preprocess::ShaderSourceInfo& _sourceInfo)
  {
    // The following code assumes the output format: ([LineNumber]) : error 0XXX: Message

    String logMsg(_shaderCompilerLogMsg);
    if (logMsg.empty())
    {
      return;
    }

    const String kLineSearchKey = ") : ";
    size_t pos = logMsg.find(kLineSearchKey);

    while (pos != std::string::npos && pos > 0u)
    {
      size_t posAfterStartBracket = pos - 1u;
      while (logMsg[posAfterStartBracket] >= '0' && logMsg[posAfterStartBracket] <= '9' )
      {
        --posAfterStartBracket;
      }
      ++posAfterStartBracket;

      String lineNumberStr = logMsg.substr(posAfterStartBracket, pos - posAfterStartBracket);
      int lineNumber = std::atoi(lineNumberStr.c_str());

      const Preprocess::FileBlock* sourceBlock = getFileBlockForLine(lineNumber, _sourceInfo);
      if (sourceBlock != nullptr)
      {
        uint32 localLineNumber = getLocalLine(lineNumber, _sourceInfo);
        String replaceString = sourceBlock->fileName + " - line " + StringUtil::toString(localLineNumber);
        logMsg.replace(posAfterStartBracket, pos - posAfterStartBracket, replaceString);
        pos += replaceString.length();
      }

      pos = logMsg.find(kLineSearchKey, pos);
    }

    log_Info(logMsg);
  }
//---------------------------------------------------------------------------//
  bool Compile::compileFromSource( const String& szSource, const Preprocess::ShaderSourceInfo& sourceInfo, const ShaderStage& eShaderStage, GpuProgramDescriptionGL4& _rDesc )
  {
    ASSERT_M(!szSource.empty(), "Invalid shader source");

    GLenum eShaderStageGL = Adapter::toGLType(eShaderStage);

    const char* szShaderSource_cstr = szSource.c_str();
    GLuint uProgramHandle = glCreateShaderProgramv(eShaderStageGL, 1, &szShaderSource_cstr);

    int iLogLengthChars = 0;
    glGetProgramiv(uProgramHandle, GL_INFO_LOG_LENGTH, &iLogLengthChars);
    ASSERT(iLogLengthChars < Internal::kMaxNumLogChars);

    char logBuffer[Internal::kMaxNumLogChars];
    if (iLogLengthChars > 0)
    {
      glGetProgramInfoLog(uProgramHandle, Internal::kMaxNumLogChars, &iLogLengthChars, logBuffer);
      outputCompilerLogMsg(logBuffer, sourceInfo);
    }

    int iProgramLinkStatus = GL_FALSE;
    glGetProgramiv(uProgramHandle, GL_LINK_STATUS, &iProgramLinkStatus);

    bool success = iProgramLinkStatus;
    if (success)
    {
      _rDesc.eShaderStage = eShaderStage;
      _rDesc.uProgramHandleGL = uProgramHandle;

      success = reflectProgram(_rDesc);
    }

    if (!success) 
    {
      log_Error(String("GpuProgram ") + _rDesc.name.toString() + " failed to compile" );
      glDeleteProgram(uProgramHandle);
    }
    
    return success;
  }
//---------------------------------------------------------------------------//
  bool Compile::reflectProgram( GpuProgramDescriptionGL4& _rDesc )
  {
    GLuint uProgramHandle = _rDesc.uProgramHandleGL;
    ASSERT(uProgramHandle != GLUINT_HANDLE_INVALID);

    if (_rDesc.eShaderStage == ShaderStage::VERTEX)
    {
      reflectVertexInputs(uProgramHandle, _rDesc.clVertexInputLayout);
    }
    else if(_rDesc.eShaderStage == ShaderStage::FRAGMENT)
    {
      reflectFragmentOutputs(uProgramHandle, _rDesc.vFragmentOutputs);
    }

    reflectConstants(uProgramHandle);
    reflectResources(uProgramHandle, _rDesc.vReadTextureInfos, _rDesc.vReadBufferInfos, _rDesc.vWriteTextureInfos, _rDesc.vWriteBufferInfos);
    return true;
  }
//---------------------------------------------------------------------------//
  void Compile::reflectResources( GLuint uProgram, 
    GpuResourceInfoList& rReadTextureInfos, GpuResourceInfoList& rReadBufferInfos, 
    GpuResourceInfoList& rWriteTextureInfos, GpuResourceInfoList& rWriteBufferInfos)
  {
    // We assume that all uniforms which don't  belong to a block are resources (i.e. buffers, samplers, textures)

    GLint numResources = 0;  // Will contain the overall number of uniforms - including block uniforms
    glGetProgramInterfaceiv(uProgram, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numResources);
    const GLenum vProperties[] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

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
      
      String szName(vPropertyValues[2], ' ');
      glGetProgramResourceName(uProgram, GL_UNIFORM, iUniform, szName.size(), nullptr, &szName[0]);

      // Construct the resourceInfo object
      GpuProgramResourceInfo resourceInfo;
      resourceInfo.name = szName;
      resourceInfo.u32RegisterIndex = vPropertyValues[3];
      Internal::getResourceTypeAndAccessTypeFromGLtype(vPropertyValues[1], 
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
  void Compile::reflectVertexInputs(GLuint uProgram, VertexInputLayout& rVertexLayout)
  {
    rVertexLayout.clear();

    GLint iNumResources = 0;
    const GLenum eInterface = GL_PROGRAM_INPUT;
    glGetProgramInterfaceiv(uProgram, eInterface, GL_ACTIVE_RESOURCES, &iNumResources);

    const GLenum vProperties[] = {GL_TYPE, GL_LOCATION};
    for (uint32 i = 0u; i < iNumResources; ++i)
    {
      GLchar _name[512] = {0u};
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
      uint32 u32ComponentCount = 1u;
      Internal::getSizeFormatFromGLtype(_type, u32SizeBytes, format, u32ComponentCount);

      ObjectName name = _name;
      VertexSemantics semantics = Internal::getVertexSemanticsFromName(name);
      
      VertexInputElement vertexElement;
      vertexElement.name = name;
      vertexElement.eSemantics = semantics;
      vertexElement.u32RegisterIndex = _location;
      vertexElement.u32SizeBytes = u32SizeBytes;
      vertexElement.eFormat = format;
      vertexElement.uFormatComponentCount = u32ComponentCount;
      
      rVertexLayout.addVertexInputElement(vertexElement);
    }
  }
//---------------------------------------------------------------------------//
  void Compile::reflectFragmentOutputs(GLuint uProgram, ShaderStageFragmentOutputList& vFragmentOutputs)
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
      uint32 _u32Size = 0u;
      uint32 componentCount = 1u;
      Internal::getSizeFormatFromGLtype(_type, _u32Size, format, componentCount);

      ShaderStageFragmentOutput output;
      output.name = _name;
      output.uRtIndex = _location;
      output.eFormat = format;
      output.uFormatComponentCount = componentCount;

      vFragmentOutputs.push_back(output);
    }
  }
//---------------------------------------------------------------------------//
  void Compile::reflectConstants( GLuint uProgram )
  {
    ShaderConstantsManager& constantsMgr = ShaderConstantsManager::getInstance();

    GLint iNumUniformBlocks = 0u;
    const GLenum eInterface = GL_UNIFORM_BLOCK;
    glGetProgramInterfaceiv(uProgram, eInterface, GL_ACTIVE_RESOURCES, &iNumUniformBlocks);
    
    for (uint32 iBlock = 0u; iBlock < iNumUniformBlocks; ++iBlock)
    {
      GLchar _name[256] = {0u};
      glGetProgramResourceName(uProgram, eInterface, iBlock, _countof(_name), nullptr, _name);
      ObjectName blockName = _name;
      ConstantBufferType eCbufferType = constantsMgr.getConstantBufferTypeFromName(blockName);
      ASSERT_M(eCbufferType != ConstantBufferType::NONE, "Invalid constant buffer name");

      //uint32 uBlockIndex = glGetProgramResourceIndex(uProgram, eInterface, _name);

      const GLenum vProperties[] = {GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE};
      GLint vPropertyValues[_countof(vProperties)] = {0x0};
      glGetProgramResourceiv(uProgram, eInterface, iBlock, _countof(vProperties), vProperties, _countof(vProperties), nullptr, vPropertyValues);

      const uint32 uUniformBlockBinding = vPropertyValues[0];
      const uint32 uNumUniformsInBlock = vPropertyValues[1];
      const uint32 uRequiredBlockSizeBytes = vPropertyValues[2];
      
      // Sanity-check of the binding point. We require it to match the eCbufferType
      ASSERT_M(uUniformBlockBinding < (uint)ConstantBufferType::NUM &&
               (ConstantBufferType) uUniformBlockBinding == eCbufferType, 
               "CBuffer-name does not match its expected binding point");

      // Allocate a backing buffer for this uniform block if necessary
      constantsMgr.registerBufferWithSize(eCbufferType, uRequiredBlockSizeBytes);

      // Acquire the indices of all active uniforms in the block
      FixedArray<GLint, kMaxNumConstantBufferElements> vUniformIndices;
      vUniformIndices.resize(uNumUniformsInBlock);

      const GLenum propActiveUniformIndices = GL_ACTIVE_VARIABLES;
      glGetProgramResourceiv(uProgram, eInterface, iBlock, 1, &propActiveUniformIndices, vUniformIndices.size(), nullptr, &vUniformIndices[0]);

      for (uint32 iUniform = 0u; iUniform < uNumUniformsInBlock; ++iUniform )
      {
        const uint32 uUniformIndex = vUniformIndices[iUniform];

        const GLenum vProperties[] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET};
        GLint vPropertyValues[_countof(vProperties)] = {0x0};
        
        glGetProgramResourceiv(uProgram, GL_UNIFORM, uUniformIndex, _countof(vProperties), 
          vProperties, _countof(vProperties), nullptr, vPropertyValues);

        GLchar _name[256] = {0u};
        ASSERT(vPropertyValues[0] <= _countof(_name));
        glGetProgramResourceName(uProgram, GL_UNIFORM, uUniformIndex, _countof(_name), nullptr, _name);

        uint32 uSizeBytes;
        DataFormat eFormat;
        uint32 uComponentCount;
        Internal::getSizeFormatFromGLtype(vPropertyValues[1], uSizeBytes, eFormat, uComponentCount);
        
        ConstantBufferElement cBufferElement;
        cBufferElement.name = _name;
        cBufferElement.uOffsetBytes = vPropertyValues[2];
        cBufferElement.eFormat = eFormat;
        cBufferElement.uSizeBytes = uSizeBytes;
        cBufferElement.uFormatComponentCount = uComponentCount;
        
        ConstantSemantics eSemantics = 
          ShaderConstantsManager::getInstance().getSemanticFromName(cBufferElement.name);

        ShaderConstantsManager::getInstance().registerElement(cBufferElement, eSemantics, eCbufferType);
      }
    }
  }
//---------------------------------------------------------------------------//
  void Compile::reflectStageInputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList)
  {
    // TODO: Implement
  }
//---------------------------------------------------------------------------//
  void Compile::reflectStageOutputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList)
  {
    // TODO: Implement
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
  bool GpuProgramCompilerGL4::compile(const String& _shaderPath, ShaderStage _eShaderStage, GpuProgramGL4& _rGpuProgram)
  {
    log_Info("Compiling shader " + _shaderPath + " ...");

    std::list<String> sourceLines;
    IO::FileReader::ReadTextFileLines(_shaderPath, sourceLines);

    if (sourceLines.empty())
    {
      log_Error("Error reading shader file " + _shaderPath);
      return false;
    }

    Preprocess::ShaderSourceInfo sourceInfo;
    Preprocess::preprocessShaderSource(_shaderPath, sourceLines, _eShaderStage, sourceInfo);

    // construct the final source string
    uint32 uRequiredLength = 0u;
    for (std::list<String>::const_iterator itLine = sourceLines.begin(); itLine != sourceLines.end(); ++itLine)
    {
      uRequiredLength += itLine->length();
    }

    String szCombinedSource("");
    szCombinedSource.reserve(uRequiredLength);
    for (std::list<String>::const_iterator itLine = sourceLines.begin(); itLine != sourceLines.end(); ++itLine)
    {
      szCombinedSource += *itLine + '\n';
    }

    GpuProgramDescriptionGL4 programDesc;
    const bool bSuccess = Compile::compileFromSource(szCombinedSource, sourceInfo, _eShaderStage, programDesc);
    if (bSuccess)
    {
      _rGpuProgram.init(programDesc);
    }
    return bSuccess;
  }
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4