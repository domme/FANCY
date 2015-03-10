
#include "ShaderConstantsManager.h"
#include "Renderer.h"
#include "Camera.h"
#include "SceneNode.h"
#include "TimeManager.h"

#include <hash_map>

#include "GpuBuffer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  ShaderConstantsUpdateStage ShaderConstantsManager::updateStage = {0u};
//---------------------------------------------------------------------------//
  namespace Internal
  {
    typedef std::hash_map<ObjectName, ConstantBufferType> ConstantBufferTypeMap;
    typedef std::hash_map<ObjectName, ConstantSemantics> ConstantSemanticsMap;
    typedef std::function<void(float*, const ShaderConstantsUpdateStage&)> ConstantUpdateFunction;

    ConstantBufferTypeMap mapConstantBufferTypes;
    ConstantSemanticsMap mapConstantSemantics;
    ConstantUpdateFunction vConstantUpdateFunctions[(uint32) ConstantSemantics::NUM];
    uint32 vConstantElementOffsets[(uint32) ConstantSemantics::NUM];

    void initialize();

    void updateTimeParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateRenderTargetSize(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateViewMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateViewInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateViewProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateViewProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateNearFarParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateCameraPosWS(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateDirLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePointLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateSpotLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateLightColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateLightPosWS(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateLightPosVS(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateDiffuseMatColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateSpecularMatColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldViewMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldViewInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldViewProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updateWorldViewProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage);
  }
//---------------------------------------------------------------------------//
  namespace Storage
  {
    /// Gpu-resident buffers representing the datastores of the constant buffers
    GpuBuffer* m_vConstantBuffers[(uint32)ConstantBufferType::NUM];
    /// Elements of the constant buffers which map each semantic to a registered element
    ConstantBufferElement m_vConstantBufferElements[(uint32)ConstantSemantics::NUM];
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void Internal::initialize()
  {
#define BEGIN_COUNTED_REGISTRY_INIT { enum {startCount = __COUNTER__};
#define REGISTER(map, key, value) ASSERT_M(map.find(key) == map.end(), "Key already registered!"); map[key] = value; __COUNTER__;
#define END_COUNTED_REGISTRY_INIT(expectedNum) static_assert((__COUNTER__ - startCount) - 1 == expectedNum, "Forgot to register some values"); }

    // Init all possible constants
    BEGIN_COUNTED_REGISTRY_INIT
      // REGISTER(mapConstantBufferTypes, _N(PER_LAUNCH), ConstantBufferType::PER_LAUNCH)
      REGISTER(mapConstantBufferTypes, _N(PER_FRAME), ConstantBufferType::PER_FRAME)
      REGISTER(mapConstantBufferTypes, _N(PER_VIEWPORT), ConstantBufferType::PER_VIEWPORT)
      // REGISTER(mapConstantBufferTypes, _N(PER_STAGE), ConstantBufferType::PER_STAGE)
      REGISTER(mapConstantBufferTypes, _N(PER_CAMERA), ConstantBufferType::PER_CAMERA) 
      REGISTER(mapConstantBufferTypes, _N(PER_LIGHT), ConstantBufferType::PER_LIGHT)
      REGISTER(mapConstantBufferTypes, _N(PER_MATERIAL), ConstantBufferType::PER_MATERIAL)
      REGISTER(mapConstantBufferTypes, _N(PER_OBJECT), ConstantBufferType::PER_OBJECT)
    END_COUNTED_REGISTRY_INIT((uint32) ConstantBufferType::NUM)
    
    BEGIN_COUNTED_REGISTRY_INIT
      // PER_LAUNCH

      // PER_FRAME
      REGISTER(mapConstantSemantics, _N(PER_FRAME.c_TimeParameters), ConstantSemantics::TIME_PARAMETERS)
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

#undef REGISTER
#define REGISTER(list, _index, value) list[_index] = value; __COUNTER__;
    
  BEGIN_COUNTED_REGISTRY_INIT
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::TIME_PARAMETERS, updateTimeParameters);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::RENDERTARGET_SIZE, updateRenderTargetSize);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::VIEW_MATRIX, updateViewMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::VIEW_INVERSE_MATRIX, updateViewInverseMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::PROJECTION_MATRIX, updateProjectionMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::PROJECTION_INVERSE_MATRIX, updateProjectionInverseMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::VIEWPROJECTION_MATRIX, updateViewProjectionMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::VIEWPROJECTION_INVERSE_MATRIX, updateViewProjectionInverseMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::NEARFAR_PARAMETERS, updateNearFarParameters);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::CAMERA_POSITION_WORLDSPACE, updateCameraPosWS);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::DIRLIGHT_PARAMETERS, updateDirLightParameters);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::POINTLIGHT_PARAMETERS, updatePointLightParameters);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::SPOTLIGHT_PARAMETERS, updateSpotLightParameters);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::LIGHT_COLORINTENSITY, updateLightColorIntensity);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::LIGHT_POSITION_WORLDSPACE, updateLightPosWS);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::LIGHT_POSITION_VIEWSPACE, updateLightPosVS);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::DIFFUSE_MATERIAL_COLORINTENSITY, updateDiffuseMatColorIntensity);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::SPECULAR_MATERIAL_COLORINTENSITY, updateSpecularMatColorIntensity);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLD_MATRIX, updateWorldMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLD_INVERSE_MATRIX, updateWorldInverseMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLDVIEW_MATRIX, updateWorldViewMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLDVIEW_INVERSE_MATRIX, updateWorldViewInverseMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLDVIEWPROJECTION_MATRIX, updateWorldViewProjectionMatrix);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantSemantics::WORLDVIEWPROJECTION_INVERSE_MATRIX, updateWorldViewProjectionInverseMatrix);
  END_COUNTED_REGISTRY_INIT((uint32) ConstantSemantics::NUM);

#undef REGISTER
#undef BEGIN_COUNTED_REGISTRY_INIT
#undef END_COUNTED_REGISTRY_INIT
  }
//---------------------------------------------------------------------------//
#pragma region Update Functions
  void Internal::updateTimeParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    _pData[0] = Time::getDeltaTime();
    _pData[1] = Time::getElapsedTime();
    _pData[2] = 0.0f;  // unused
    _pData[3] = 0.0f;  // unused
  }
//---------------------------------------------------------------------------//
  void Internal::updateRenderTargetSize(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pRenderer);
    const glm::uvec4& uvViewportParams = _updateStage.pRenderer->getViewport();
    _pData[0] = uvViewportParams.z;  // width
    _pData[1] = uvViewportParams.w;  // height
    _pData[2] = 1.0f / _pData[0]; // 1 / width
    _pData[3] = 1.0f / _pData[1];  // 1 / height
  }
//---------------------------------------------------------------------------//
  void Internal::updateViewMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewMat = _updateStage.pCamera->getView();
    memcpy(_pData, glm::value_ptr(viewMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateViewInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewInvMat = _updateStage.pCamera->getViewInv();
    memcpy(_pData, glm::value_ptr(viewInvMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     const glm::mat4& projMat = _updateStage.pCamera->getProjection();
     memcpy(_pData, glm::value_ptr(projMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     glm::mat4 projInvMat(glm::inverse(_updateStage.pCamera->getProjection()));
     memcpy(_pData, glm::value_ptr(projInvMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateViewProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
     memcpy(_pData, glm::value_ptr(viewProjMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateViewProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
     glm::mat4 viewProjInv(glm::inverse(viewProjMat));
     memcpy(_pData, glm::value_ptr(viewProjInv), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateNearFarParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     const float fNear = _updateStage.pCamera->getNearPlane();
     const float fFar = _updateStage.pCamera->getFarPlane();
     _pData[0] = fNear;
     _pData[1] = fFar;
     _pData[2] = fNear / fFar;
     _pData[3] = 1.0f / fFar;
  }
//---------------------------------------------------------------------------//
  void Internal::updateCameraPosWS(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
     ASSERT(_updateStage.pCamera);
     const glm::mat4& viewInvMat = _updateStage.pCamera->getViewInv();
     const glm::vec4 camPosWS = glm::column(viewInvMat, 3);
     memcpy(_pData, glm::value_ptr(camPosWS), sizeof(glm::vec4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateDirLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updatePointLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updateSpotLightParameters(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updateLightColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updateLightPosWS(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updateLightPosVS(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement lights
  }
//---------------------------------------------------------------------------//
  void Internal::updateDiffuseMatColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement Material params
  }
//---------------------------------------------------------------------------//
  void Internal::updateSpecularMatColorIntensity(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    // TODO: Implement Material params
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    memcpy(_pData, glm::value_ptr(worldMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldInvMat(glm::affineInverse(worldMat));
    memcpy(_pData, glm::value_ptr(worldInvMat), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldViewMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewMat = _updateStage.pCamera->getView();
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldView(viewMat * worldMat);
    memcpy(_pData, glm::value_ptr(worldView), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldViewInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewMat = _updateStage.pCamera->getView();
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldViewInv(glm::affineInverse(viewMat * worldMat));
    memcpy(_pData, glm::value_ptr(worldViewInv), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldViewProjectionMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldViewProj(viewProjMat * worldMat);
    memcpy(_pData, glm::value_ptr(worldViewProj), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
  void Internal::updateWorldViewProjectionInverseMatrix(float* _pData, const ShaderConstantsUpdateStage& _updateStage)
  {
    ASSERT(_updateStage.pWorldMat);
    ASSERT(_updateStage.pCamera);
    const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldViewProjInv(glm::inverse(viewProjMat * worldMat));
    memcpy(_pData, glm::value_ptr(worldViewProjInv), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
#pragma endregion
//---------------------------------------------------------------------------//
  ShaderConstantsManager::ShaderConstantsManager()
  {
    memset(Storage::m_vConstantBuffers, 0x0, sizeof(Storage::m_vConstantBuffers));
    memset(Storage::m_vConstantBufferElements, 0x0, sizeof(Storage::m_vConstantBufferElements));
    memset(Internal::vConstantElementOffsets, -1u, sizeof(Internal::vConstantElementOffsets));

    Internal::initialize();

    updateStage.pRenderer = &Renderer::getInstance();
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
      { ConstantSemantics::PER_FRAME_BEGIN, 
       ConstantSemantics::PER_VIEWPORT_BEGIN, 
       ConstantSemantics::PER_CAMERA_BEGIN, 
       ConstantSemantics::PER_MATERIAL_BEGIN, 
       ConstantSemantics::PER_DRAW_BEGIN };

    static const ConstantSemantics semanticsEnd[] = 
      { ConstantSemantics::PER_FRAME_END, 
      ConstantSemantics::PER_VIEWPORT_END,
      ConstantSemantics::PER_CAMERA_END, 
      ConstantSemantics::PER_MATERIAL_END,
      ConstantSemantics::PER_DRAW_END };

    const uint32 semanticFrom = (uint32) semanticsBegin[(uint32)eType];
    const uint32 semanticTo = (uint32) semanticsEnd[(uint32)eType];
    
    uint8* const pConstantData = static_cast<uint8*>(Storage::m_vConstantBuffers[(uint32)eType]->lock(GpuResoruceLockOption::WRITE_PERSISTENT_COHERENT));
    ASSERT(pConstantData);

    for (uint32 i = semanticFrom; i < semanticTo; ++i)
    {
      // TODO: Should we deprecate the elements and only work with the full layout as defined in the shader?
      // const ConstantBufferElement& element = Storage::m_vConstantBufferElements[i];
      
      const uint32 offset = Internal::vConstantElementOffsets[i];
      ASSERT_M(offset != -1u, "Trying to update an element that was not registered before (i.e. not encountered in any shader). Is it really needed?");
      const Internal::ConstantUpdateFunction& updateFunction = Internal::vConstantUpdateFunctions[i];
      updateFunction((float*) (pConstantData + offset), updateStage);
    }
    Storage::m_vConstantBuffers[(uint32)eType]->unlock();
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
                                | (uint32)GpuResourceAccessFlags::COHERENT
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      // bufferParams.bIsMultiBuffered = true;
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
    Internal::vConstantElementOffsets[uSemanticIdx] = element.uOffsetBytes;
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::bindBuffers(Rendering::Renderer* _pRenderer)
  {
    for (uint32 iShaderStage = 0u; iShaderStage < (uint32) ShaderStage::NUM; ++iShaderStage)
    {
      const ShaderStage eShaderStage = (ShaderStage) iShaderStage;

      for (uint32 iConstantBuffer = 0u; iConstantBuffer < (uint32) ConstantBufferType::NUM; ++iConstantBuffer)
      {
        const GpuBuffer* pConstantBuffer = Storage::m_vConstantBuffers[iConstantBuffer];
        _pRenderer->setConstantBuffer(pConstantBuffer, eShaderStage, iConstantBuffer);
      }
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering