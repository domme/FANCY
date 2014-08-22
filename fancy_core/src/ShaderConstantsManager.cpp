
#include "ShaderConstantsManager.h"

#include "GpuBuffer.h"

namespace Fancy { namespace Core { namespace Rendering {
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  ShaderConstantsManager::ShaderConstantsManager()
  {
    memset(m_vConstantBuffers, 0x0, sizeof(m_vConstantBuffers));
  }
//---------------------------------------------------------------------------//
  ShaderConstantsManager::~ShaderConstantsManager()
  {
    for (uint32 i = 0; i < _countof(m_vConstantBuffers); ++i)
    {
      if (m_vConstantBuffers[i] != nullptr) 
      {
        FANCY_DELETE(m_vConstantBuffers[i], MemoryCategory::BUFFERS);
        m_vConstantBuffers[i] = nullptr;
      }
    }
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::updateConstants( ConstantBufferType eType )
  {
    ASSERT(m_vConstantBuffers[(uint32)eType]);
    
    static const ConstantSemantics semanticsBegin[] = 
      { ConstantSemantics::PER_LAUNCH_BEGIN, ConstantSemantics::PER_FRAME_BEGIN, 
       ConstantSemantics::PER_VIEWPORT_BEGIN, ConstantSemantics::PER_STAGE_BEGIN, 
       ConstantSemantics::PER_CAMERA_BEGIN, ConstantSemantics::PER_MATERIAL_BEGIN, 
       ConstantSemantics::PER_DRAW_BEGIN };

    static const ConstantSemantics semanticsEnd[] = 
      { ConstantSemantics::PER_LAUNCH_END, ConstantSemantics::PER_FRAME_END, 
      ConstantSemantics::PER_VIEWPORT_END, ConstantSemantics::PER_STAGE_END, 
      ConstantSemantics::PER_CAMERA_END, ConstantSemantics::PER_MATERIAL_END, 
      ConstantSemantics::PER_DRAW_END };

    const uint32 semanticFrom = (uint32) semanticsBegin[(uint32)eType];
    const uint32 semanticTo = (uint32) semanticsEnd[(uint32)eType];
    
    void* const pConstantData = 
      m_vConstantBuffers[(uint32)eType]->lock(GpuResoruceLockOption::WRITE_PERSISTENT);
    ASSERT(pConstantData);

    for (uint32 i = semanticFrom; i < semanticTo; ++i)
    {
      const ConstantBufferElement& element = m_vConstantBufferElements[i];
      updateElement(element, static_cast<ConstantSemantics>(i), pConstantData);
    }
    m_vConstantBuffers[(uint32)eType]->unlock();
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::
    updateElement(const ConstantBufferElement& element, ConstantSemantics eElementSemantic, void* const pBufferData)
  {
    const float* pElementData = nullptr;
    
    switch(eElementSemantic)
    {
      case ConstantSemantics::NONE: 
        {

        } break;
      case ConstantSemantics::RENDERTARGET_SIZE:
        {

        } break;
      case ConstantSemantics::VIEW_MATRIX:
        {

        } break;
      case ConstantSemantics::VIEW_INVERSE_MATRIX:
        {

        } break;
      case ConstantSemantics::PROJECTION_MATRIX:
        {

        } break;
      case ConstantSemantics::PROJECTION_INVERSE_MATRIX:
        {

        } break;
      case ConstantSemantics::VIEWPROJECTION_MATRIX:
        {

        } break;
      case ConstantSemantics::VIEWPROJECTION_INVERSE_MATRIX:
        {

        } break;
      case ConstantSemantics::NEARFAR_PARAMETERS:
        {

        } break;
      case ConstantSemantics::CAMERA_POSITION_WORLDSPACE:
        {

        } break;
      case ConstantSemantics::DIRLIGHT_PARAMETERS:
        {

        } break;
      case ConstantSemantics::POINTLIGHT_PARAMETERS:
        {

        } break;
      case ConstantSemantics::SPOTLIGHT_PARAMETERS:
        {

        } break;
      case ConstantSemantics::LIGHT_COLORINTENSITY:
        {

        } break;
      case ConstantSemantics::LIGHT_POSITION_WORLDSPACE:
        {

        } break;
      case ConstantSemantics::LIGHT_POSITION_VIEWSPACE:
        {

        } break;
      case ConstantSemantics::DIFFUSE_MATERIAL_COLORINTENSITY:
        {

        } break;
      case ConstantSemantics::SPECULAR_MATERIAL_COLORINTENSITY:
        {

        } break;
      case ConstantSemantics::WORLD_MATRIX:
        {

        } break;
      case ConstantSemantics::WORLD_INVERSE_MATRIX:
        {

        } break;
      case ConstantSemantics::WORLDVIEW_MATRIX:
        {

        } break;
      case ConstantSemantics::WORLDVIEW_INVERSE_MATRIX:
        {

        } break;
      case ConstantSemantics::WORLDVIEWPROJECTION_MATRIX:
        {

        } break;
      case ConstantSemantics::WORLDVIEWPROJECTION_INVERSE_MATRIX:
        {

        } break;
      default:
        {
          ASSERT_M(false, "Shader semantic not implemented");
        } break;
    }  // end switch semantics

    // Copy the value into the buffer
    memcpy(static_cast<uint8* const>(pBufferData) + element.uOffsetBytes,
           pElementData, 
           element.uSizeBytes);
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::
    registerElement( const ConstantBufferElement& element, ConstantSemantics eElementSemantic )
  {
    const uint32 uSemanticIdx = static_cast<uint32>(eElementSemantic);
    ASSERT_M(m_vConstantBufferElements[uSemanticIdx].uSizeBytes != 0u, 
      "Constant buffer element already registered");

    m_vConstantBufferElements[uSemanticIdx] = element;

    // determine the required update-rate and allocate a buffer when needed
    ConstantBufferType eBufferType = ConstantBufferType::NONE;

    if (eElementSemantic > ConstantSemantics::PER_LAUNCH_BEGIN &&
        eElementSemantic < ConstantSemantics::PER_LAUNCH_END) 
    {
        eBufferType = ConstantBufferType::PER_LAUNCH;
    }
    else if (eElementSemantic > ConstantSemantics::PER_FRAME_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_FRAME_END) 
    {
        eBufferType = ConstantBufferType::PER_FRAME;
    }
    else if (eElementSemantic > ConstantSemantics::PER_VIEWPORT_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_VIEWPORT_END) 
    {
         eBufferType = ConstantBufferType::PER_VIEWPORT;
    }
    else if (eElementSemantic > ConstantSemantics::PER_STAGE_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_STAGE_END) 
    {
        eBufferType = ConstantBufferType::PER_STAGE;
    }
    else if (eElementSemantic > ConstantSemantics::PER_CAMERA_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_CAMERA_END) 
    {
        eBufferType = ConstantBufferType::PER_CAMERA;
    }
    else if (eElementSemantic > ConstantSemantics::PER_MATERIAL_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_MATERIAL_END) 
    {
        eBufferType = ConstantBufferType::PER_MATERIAL;
    }
    else if (eElementSemantic > ConstantSemantics::PER_DRAW_BEGIN &&
             eElementSemantic < ConstantSemantics::PER_DRAW_END) 
    {
      eBufferType = ConstantBufferType::PER_DRAW;
    }
    
    const uint32 uBufferTypeIdx = static_cast<uint32>(eBufferType);
    const bool bufferAllocationNeeded = m_vConstantBuffers[uBufferTypeIdx] == nullptr;
    if (bufferAllocationNeeded)
    {
      GpuBuffer* const pBuffer = FANCY_NEW(GpuBuffer, MemoryCategory::BUFFERS);

      GpuBufferParameters bufferParams;
      bufferParams.ePrimaryUsageType = GpuBufferUsage::CONSTANT_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::WRITE 
                                | (uint32)GpuResourceAccessFlags::DYNAMIC 
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uNumElements = kNumConstantBufferFloats * internal::kNumMultiBuffers;
      bufferParams.uElementSizeBytes = sizeof(float);

      pBuffer->create(bufferParams);

      m_vConstantBuffers[uBufferTypeIdx] = pBuffer;
    }
  }
//---------------------------------------------------------------------------//
} } } 
