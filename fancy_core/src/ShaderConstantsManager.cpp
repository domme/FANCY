
#include "ShaderConstantsManager.h"

#include <hash_map>

#include "GpuBuffer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace internal
  {
    typedef std::hash_map<ObjectName, ConstantBufferType> ConstantBufferTypeMap;
    typedef std::hash_map<ObjectName, ConstantSemantics> ConstantSemanticsMap;

    ConstantBufferTypeMap mapConstantBufferTypes;
    ConstantSemanticsMap mapConstantSemantics;
    
    void initialize();
    ConstantBufferType getConstantBufferTypeFromSemantics(ConstantSemantics eSemantic);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void internal::initialize()
  {
#define BEGIN_COUNTED_REGISTRY_INIT { enum {startCount = __COUNTER__};
#define REGISTER(map, key, value) ASSERT_M(map.find(key) == map.end(), "Key already registered!"); map[key] = value; __COUNTER__;
#define END_COUNTED_REGISTRY_INIT(expectedNum) static_assert(__COUNTER__ - startCount == expectedNum, "Forgot to register some values"); }

    // Init all possible constants
    BEGIN_COUNTED_REGISTRY_INIT
      REGISTER(mapConstantBufferTypes, _N(PER_LAUNCH), ConstantBufferType::PER_LAUNCH)
      REGISTER(mapConstantBufferTypes, _N(PER_FRAME), ConstantBufferType::PER_FRAME)
      REGISTER(mapConstantBufferTypes, _N(PER_VIEWPORT), ConstantBufferType::PER_VIEWPORT)
      REGISTER(mapConstantBufferTypes, _N(PER_STAGE), ConstantBufferType::PER_STAGE)
      REGISTER(mapConstantBufferTypes, _N(PER_CAMERA), ConstantBufferType::PER_CAMERA) 
      REGISTER(mapConstantBufferTypes, _N(PER_LIGHT), ConstantBufferType::PER_LIGHT)
      REGISTER(mapConstantBufferTypes, _N(PER_MATERIAL), ConstantBufferType::PER_MATERIAL)
      REGISTER(mapConstantBufferTypes, _N(PER_OBJECT), ConstantBufferType::PER_OBJECT)
    END_COUNTED_REGISTRY_INIT((uint32) ConstantBufferType::NUM)
    
    BEGIN_COUNTED_REGISTRY_INIT
      // PER_LAUNCH

      // PER_FRAME

      // PER_VIEWPORT
      REGISTER(mapConstantSemantics, _N(PER_VIEWPORT.c_RenderTargetSize), ConstantSemantics::RENDERTARGET_SIZE)

      // PER_STAGE
      
      // PER_CAMERA
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ViewMatrix), ConstantSemantics::VIEW_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ViewInverseMatrix), ConstantSemantics::VIEW_INVERSE_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ProjectionMatrix), ConstantSemantics::PROJECTION_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ProjectionInverseMatrix), ConstantSemantics::PROJECTION_INVERSE_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ViewProjectionMatrix), ConstantSemantics::VIEWPROJECTION_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_ViewProjectionInverseMatrix), ConstantSemantics::VIEWPROJECTION_INVERSE_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_NearFarParameters), ConstantSemantics::NEARFAR_PARAMETERS) 
      REGISTER(mapConstantSemantics, _N(PER_CAMERA.c_CameraPosWS), ConstantSemantics::CAMERA_POSITION_WORLDSPACE)

      // PER_LIGHT
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_DirLightParameters), ConstantSemantics::DIRLIGHT_PARAMETERS)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_PointLightParameters), ConstantSemantics::POINTLIGHT_PARAMETERS)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_SpotLightParameters), ConstantSemantics::SPOTLIGHT_PARAMETERS)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightColorIntensity), ConstantSemantics::LIGHT_COLORINTENSITY)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightPosWS), ConstantSemantics::LIGHT_POSITION_WORLDSPACE)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightPosVS), ConstantSemantics::LIGHT_POSITION_VIEWSPACE)

      // PER_MATERIAL
      REGISTER(mapConstantSemantics, _N(PER_MATERIAL.c_MatDiffIntensity), ConstantSemantics::DIFFUSE_MATERIAL_COLORINTENSITY)
      REGISTER(mapConstantSemantics, _N(PER_MATERIAL.c_MatSpecIntensity), ConstantSemantics::SPECULAR_MATERIAL_COLORINTENSITY)

      // PER_DRAW
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldMatrix), ConstantSemantics::WORLD_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldInverseMatrix), ConstantSemantics::WORLD_INVERSE_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldViewMatrix), ConstantSemantics::WORLDVIEW_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldViewInverseMatrix), ConstantSemantics::WORLDVIEW_INVERSE_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldViewProjectionMatrix), ConstantSemantics::WORLDVIEWPROJECTION_MATRIX)
      REGISTER(mapConstantSemantics, _N(PER_OBJECT.c_WorldViewProjectionInverseMatrix), ConstantSemantics::WORLDVIEWPROJECTION_INVERSE_MATRIX)
    END_COUNTED_REGISTRY_INIT((uint32) ConstantSemantics::NUM)

#undef BEGIN_COUNTED_REGISTRY_INIT
#undef REGISTER
#undef END_COUNTED_REGISTRY_INIT
  }
//---------------------------------------------------------------------------//
  ConstantBufferType internal::getConstantBufferTypeFromSemantics(ConstantSemantics eSemantic)
  {
    ConstantBufferType eBufferType = ConstantBufferType::NONE;

    if (eSemantic > ConstantSemantics::PER_LAUNCH_BEGIN &&
      eSemantic < ConstantSemantics::PER_LAUNCH_END) 
    {
      eBufferType = ConstantBufferType::PER_LAUNCH;
    }
    else if (eSemantic > ConstantSemantics::PER_FRAME_BEGIN &&
      eSemantic < ConstantSemantics::PER_FRAME_END) 
    {
      eBufferType = ConstantBufferType::PER_FRAME;
    }
    else if (eSemantic > ConstantSemantics::PER_VIEWPORT_BEGIN &&
      eSemantic < ConstantSemantics::PER_VIEWPORT_END) 
    {
      eBufferType = ConstantBufferType::PER_VIEWPORT;
    }
    else if (eSemantic > ConstantSemantics::PER_STAGE_BEGIN &&
      eSemantic < ConstantSemantics::PER_STAGE_END) 
    {
      eBufferType = ConstantBufferType::PER_STAGE;
    }
    else if (eSemantic > ConstantSemantics::PER_CAMERA_BEGIN &&
      eSemantic < ConstantSemantics::PER_CAMERA_END) 
    {
      eBufferType = ConstantBufferType::PER_CAMERA;
    }
    else if (eSemantic > ConstantSemantics::PER_MATERIAL_BEGIN &&
      eSemantic < ConstantSemantics::PER_MATERIAL_END) 
    {
      eBufferType = ConstantBufferType::PER_MATERIAL;
    }
    else if (eSemantic > ConstantSemantics::PER_DRAW_BEGIN &&
      eSemantic < ConstantSemantics::PER_DRAW_END) 
    {
      eBufferType = ConstantBufferType::PER_OBJECT;
    }

    return eBufferType;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  ShaderConstantsManager::ShaderConstantsManager()
  {
    memset(m_vConstantBuffers, 0x0, sizeof(m_vConstantBuffers));
    memset(m_vConstantBufferElements, 0x0, sizeof(m_vConstantBufferElements));

    internal::initialize();
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
  ConstantSemantics ShaderConstantsManager::getSemanticFromName( const ObjectName& clName )
  {
    internal::ConstantSemanticsMap::const_iterator it = internal::mapConstantSemantics.find(clName);
    ASSERT(it != internal::mapConstantSemantics.end());
    if(it != internal::mapConstantSemantics.end())
    {
      return (*it).second;
    }

    return ConstantSemantics::NONE;
  }
//---------------------------------------------------------------------------//
  Fancy::Rendering::ConstantBufferType ShaderConstantsManager::getConstantBufferTypeFromName( const ObjectName& clName )
  {
    internal::ConstantBufferTypeMap::const_iterator it = internal::mapConstantBufferTypes.find(clName);
    ASSERT(it != internal::mapConstantBufferTypes.end());
    if (it != internal::mapConstantBufferTypes.end())
    {
      return (*it).second;
    }

    return ConstantBufferType::NONE;
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
    registerElement( const ConstantBufferElement& element, ConstantSemantics eElementSemantic, ConstantBufferType eConstantBufferType )
  {
    // Sanity-check on the detected update rate vs passed buffertype
    ASSERT_M(internal::getConstantBufferTypeFromSemantics(eElementSemantic) == eConstantBufferType,
      "Mismatch between the constant-semantic and the requested buffer type");

    const uint32 uSemanticIdx = static_cast<uint32>(eElementSemantic);
    if (m_vConstantBufferElements[uSemanticIdx].uSizeBytes > 0u) 
    {
      // Element is already registered 
      return;
    }
    m_vConstantBufferElements[uSemanticIdx] = element;

    // Allocate the constant buffer storage if needed
    const uint32 uBufferTypeIdx = static_cast<uint32>(eConstantBufferType);
    if (m_vConstantBuffers[uBufferTypeIdx] == nullptr)
    {
      GpuBuffer* const pBuffer = FANCY_NEW(GpuBuffer, MemoryCategory::BUFFERS);

      GpuBufferParameters bufferParams;
      bufferParams.ePrimaryUsageType = GpuBufferUsage::CONSTANT_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::WRITE 
                                | (uint32)GpuResourceAccessFlags::DYNAMIC 
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.bIsMultiBuffered = true;
      bufferParams.uNumElements = kNumConstantBufferFloats;
      bufferParams.uElementSizeBytes = sizeof(float);

      pBuffer->create(bufferParams);

      m_vConstantBuffers[uBufferTypeIdx] = pBuffer;
    }
  }
//---------------------------------------------------------------------------//
} } 
