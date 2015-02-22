
#include "ShaderConstantsManager.h"

#include <hash_map>

#include "GpuBuffer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  namespace Internal
  {
    typedef std::hash_map<ObjectName, ConstantBufferType> ConstantBufferTypeMap;
    typedef std::hash_map<ObjectName, ConstantSemantics> ConstantSemanticsMap;

    ConstantBufferTypeMap mapConstantBufferTypes;
    ConstantSemanticsMap mapConstantSemantics;
    
    void initialize();
  }
//---------------------------------------------------------------------------//
  namespace Storage
  {
    void updateElement(const ConstantBufferElement& element, ConstantSemantics eElementSemantic, void* const pBufferData);

    /// Gpu-resident buffers representing the datastores of the constant buffers
    GpuBuffer* m_vConstantBuffers[(uint32)ConstantBufferType::NUM];
    /// Elements of the constant buffers which map each semantic to a registered element
    ConstantBufferElement m_vConstantBufferElements[(uint32)ConstantSemantics::NUM];
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void Internal::initialize()
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
  ShaderConstantsManager::ShaderConstantsManager()
  {
    memset(Storage::m_vConstantBuffers, 0x0, sizeof(Storage::m_vConstantBuffers));
    memset(Storage::m_vConstantBufferElements, 0x0, sizeof(Storage::m_vConstantBufferElements));

    Internal::initialize();
  }
//---------------------------------------------------------------------------//
  ShaderConstantsManager::~ShaderConstantsManager()
  {
    for (uint32 i = 0; i < _countof(Storage::m_vConstantBuffers); ++i)
    {
      if (Storage::m_vConstantBuffers[i] != nullptr) 
      {
        FANCY_DELETE(Storage::m_vConstantBuffers[i], MemoryCategory::BUFFERS);
        Storage::m_vConstantBuffers[i] = nullptr;
      }
    }
  }  
//---------------------------------------------------------------------------//
  bool ShaderConstantsManager::hasBackingBuffer(ConstantBufferType eType)
  {
    return Storage::m_vConstantBuffers[(uint32)eType] != nullptr;
  }
//---------------------------------------------------------------------------//
  ConstantSemantics ShaderConstantsManager::getSemanticFromName( const ObjectName& clName )
  {
    Internal::ConstantSemanticsMap::const_iterator it = Internal::mapConstantSemantics.find(clName);
    ASSERT(it != Internal::mapConstantSemantics.end());
    if(it != Internal::mapConstantSemantics.end())
    {
      return (*it).second;
    }

    return ConstantSemantics::NONE;
  }
//---------------------------------------------------------------------------//
  Fancy::Rendering::ConstantBufferType ShaderConstantsManager::getConstantBufferTypeFromName( const ObjectName& clName )
  {
    Internal::ConstantBufferTypeMap::const_iterator it = Internal::mapConstantBufferTypes.find(clName);
    ASSERT(it != Internal::mapConstantBufferTypes.end());
    if (it != Internal::mapConstantBufferTypes.end())
    {
      return (*it).second;
    }

    return ConstantBufferType::NONE;
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::update( ConstantBufferType eType )
  {
    ASSERT(Storage::m_vConstantBuffers[(uint32)eType]);
    
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
      Storage::m_vConstantBuffers[(uint32)eType]->lock(GpuResoruceLockOption::WRITE_PERSISTENT);
    ASSERT(pConstantData);

    for (uint32 i = semanticFrom; i < semanticTo; ++i)
    {
      const ConstantBufferElement& element = Storage::m_vConstantBufferElements[i];
      Storage::updateElement(element, static_cast<ConstantSemantics>(i), pConstantData);
    }
    Storage::m_vConstantBuffers[(uint32)eType]->unlock();
  }
//---------------------------------------------------------------------------//
  void Storage::updateElement(const ConstantBufferElement& element, ConstantSemantics eElementSemantic, void* const pBufferData)
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
  void ShaderConstantsManager::registerBufferWithSize(ConstantBufferType _eConstantBufferType, uint32 _requiredSizeBytes)
  {
    // Allocate the constant buffer storage if needed
    const uint32 uBufferTypeIdx = static_cast<uint32>(_eConstantBufferType);
    if (Storage::m_vConstantBuffers[uBufferTypeIdx] == nullptr)
    {
      GpuBuffer* const pBuffer = FANCY_NEW(GpuBuffer, MemoryCategory::BUFFERS);

      GpuBufferParameters bufferParams;
      bufferParams.ePrimaryUsageType = GpuBufferUsage::CONSTANT_BUFFER;
      bufferParams.uAccessFlags = (uint32)GpuResourceAccessFlags::WRITE 
                                | (uint32)GpuResourceAccessFlags::DYNAMIC 
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.bIsMultiBuffered = true;
      // TODO: Currently, we assume that all elements of the constant buffer are floats.
      bufferParams.uNumElements = _requiredSizeBytes / sizeof(float);
      bufferParams.uElementSizeBytes = sizeof(float);

      pBuffer->create(bufferParams);

      Storage::m_vConstantBuffers[uBufferTypeIdx] = pBuffer;
    }
    
    ASSERT_M(Storage::m_vConstantBuffers[uBufferTypeIdx]->getTotalSizeBytes() == _requiredSizeBytes, "Requested the same constant buffer with two different sizes");
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::
    registerElement( const ConstantBufferElement& element, ConstantSemantics eElementSemantic, ConstantBufferType eConstantBufferType )
  {
    const uint32 uSemanticIdx = static_cast<uint32>(eElementSemantic);
    if (Storage::m_vConstantBufferElements[uSemanticIdx].uSizeBytes > 0u) 
    {
      // Element is already registered 
      return;
    }
    Storage::m_vConstantBufferElements[uSemanticIdx] = element;
  }
//---------------------------------------------------------------------------//
} } 
