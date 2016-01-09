
#include "ShaderConstantsManager.h"
#include "Renderer.h"
#include "SceneNode.h"
#include "TimeManager.h"
#include "LightComponent.h"
#include "GpuProgramResource.h"

#include <unordered_map>

#include "GpuBuffer.h"
#include <corecrt_memcpy_s.h>
#include <stdlib.h>

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  ShaderConstantsUpdateStage ShaderConstantsManager::updateStage = {0u};
//---------------------------------------------------------------------------//
  namespace Internal
  {
    typedef std::unordered_map<uint32, ConstantBufferType> ConstantBufferTypeMap;
    typedef std::unordered_map<uint32, ConstantSemantics> ConstantSemanticsMap;
    typedef std::function<void(uint8*, const ShaderConstantsUpdateStage&)> ConstantUpdateFunction;

    ConstantBufferTypeMap mapConstantBufferTypes;
    ConstantSemanticsMap mapConstantSemantics;
    ConstantUpdateFunction vConstantUpdateFunctions[(uint32) ConstantSemantics::NUM];
    uint32 vConstantElementOffsets[(uint32) ConstantSemantics::NUM];

    void initialize();

    void updatePerLaunchData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerViewportData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerFrameData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerStageData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerCameraData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerLightData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerMaterialData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
    void updatePerObjectData(uint8* _pData, const ShaderConstantsUpdateStage& _updateStage);
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
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightParameters), ConstantSemantics::LIGHT_PARAMETERS)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_PointSpotParameters), ConstantSemantics::LIGHT_POINTSPOT_PARAMETERS)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightPosWS), ConstantSemantics::LIGHT_POSITION_WORLDSPACE)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightPosVS), ConstantSemantics::LIGHT_POSITION_VIEWSPACE)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightDirWS), ConstantSemantics::LIGHT_DIRECTION_WORLDSPACE)
      REGISTER(mapConstantSemantics, _N(PER_LIGHT.c_LightDirVS), ConstantSemantics::LIGHT_DIRECTION_VIEWSPACE)

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
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_FRAME, updatePerFrameData);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_VIEWPORT, updatePerViewportData);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_CAMERA, updatePerCameraData);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_LIGHT, updatePerLightData);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_MATERIAL, updatePerMaterialData);
    REGISTER(vConstantUpdateFunctions, (uint32) ConstantBufferType::PER_OBJECT, updatePerObjectData);
  END_COUNTED_REGISTRY_INIT((uint32) ConstantBufferType::NUM);

#undef REGISTER
#undef BEGIN_COUNTED_REGISTRY_INIT
#undef END_COUNTED_REGISTRY_INIT
  }
//---------------------------------------------------------------------------//
#pragma region Update Functions

#define BEGIN_CBUFFER_UPDATE { enum {startCount = __COUNTER__};
#define GET_ELEMENT_PTR(type) (float*) (_pData + vConstantElementOffsets[(uint32) type]); \
  ASSERT_M(vConstantElementOffsets[(uint32) type] != -1, "Trying to update an element that was not registered before (i.e. not encountered in any shader). Is it really needed?"); __COUNTER__;
#define END_CBUFFER_UPDATE(expectedNum) static_assert((__COUNTER__ - startCount) - 1 == expectedNum, "Forgot to update some elements"); }

  void Internal::updatePerViewportData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    ASSERT(_updateStage.pRenderer);
    float* pData = GET_ELEMENT_PTR(ConstantSemantics::RENDERTARGET_SIZE);
    const glm::uvec4& uvViewportParams = _updateStage.pRenderer->getViewport();
    pData[0] = uvViewportParams.z;  // width
    pData[1] = uvViewportParams.w;  // height
    pData[2] = 1.0f / pData[0]; // 1 / width
    pData[3] = 1.0f / pData[1];  // 1 / height
  }
//---------------------------------------------------------------------------//
  void Internal::updatePerFrameData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    float* pData = GET_ELEMENT_PTR(ConstantSemantics::TIME_PARAMETERS);
    pData[0] = Time::getDeltaTime();
    pData[1] = Time::getElapsedTime();
    pData[2] = 0.0f;  // unused
    pData[3] = 0.0f;  // unused
  }
//---------------------------------------------------------------------------//
  void Internal::updatePerStageData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {

  }
//---------------------------------------------------------------------------//
  void Internal::updatePerCameraData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    ASSERT(_updateStage.pCamera);

    const glm::mat4& viewMat = _updateStage.pCamera->getView();
    const glm::mat4& viewInvMat = _updateStage.pCamera->getViewInv();
    const glm::mat4& projMat = _updateStage.pCamera->getProjection();
    const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
    glm::mat4 projInvMat(glm::inverse(projMat));
    glm::mat4 viewProjInv(glm::inverse(viewProjMat));
    const float fNear = _updateStage.pCamera->getNearPlane();
    const float fFar = _updateStage.pCamera->getFarPlane();

    float* pData = GET_ELEMENT_PTR(ConstantSemantics::VIEW_MATRIX);
    memcpy(pData, glm::value_ptr(viewMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::VIEW_INVERSE_MATRIX);
    memcpy(pData, glm::value_ptr(viewInvMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::PROJECTION_MATRIX);
    memcpy(pData, glm::value_ptr(projMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::PROJECTION_INVERSE_MATRIX);
    memcpy(pData, glm::value_ptr(projInvMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::VIEWPROJECTION_MATRIX);
    memcpy(pData, glm::value_ptr(viewProjMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::VIEWPROJECTION_INVERSE_MATRIX);
    memcpy(_pData, glm::value_ptr(viewProjInv), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::NEARFAR_PARAMETERS);
    pData[0] = fNear;
    pData[1] = fFar;
    pData[2] = fNear / fFar;
    pData[3] = 1.0f / fFar;

    pData = GET_ELEMENT_PTR(ConstantSemantics::NEARFAR_PARAMETERS);
    const glm::vec4 camPosWS = glm::column(viewInvMat, 3);
    memcpy(pData, glm::value_ptr(camPosWS), sizeof(glm::vec4));
  }
//---------------------------------------------------------------------------//
  void Internal::updatePerLightData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    ASSERT(_updateStage.pLight);
    ASSERT(_updateStage.pCamera);

    const Scene::LightComponent* pLight = _updateStage.pLight;
    const Scene::CameraComponent* pCamera = _updateStage.pCamera;
    const Scene::Transform& lightTransform = pLight->getSceneNode()->getTransform();

    glm::vec4 lightParams(pLight->getColorIntensity(), static_cast<float>(pLight->getType()));
    glm::vec4 pointSpotParams(pLight->getFalloffStart(), pLight->getFalloffEnd(), pLight->getConeAngle(), 0.0f);
    glm::vec3 lightPosVS = static_cast<glm::vec3>(pCamera->getView() * glm::vec4(lightTransform.getPosition(), 1.0f));
    glm::vec3 lightDirVS = glm::normalize(static_cast<glm::mat3>(pCamera->getView()) * lightTransform.forward());
    
    float* pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_PARAMETERS);
    memcpy(pData, glm::value_ptr(lightParams), sizeof(glm::vec4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_POINTSPOT_PARAMETERS);
    memcpy(pData, glm::value_ptr(pointSpotParams), sizeof(glm::vec4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_POSITION_WORLDSPACE);
    memcpy(pData, glm::value_ptr(lightTransform.getPosition()), sizeof(glm::vec3));

    pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_POSITION_VIEWSPACE);
    memcpy(pData, glm::value_ptr(lightPosVS), sizeof(glm::vec3));

    pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_DIRECTION_WORLDSPACE);
    memcpy(pData, glm::value_ptr(lightTransform.forward()), sizeof(glm::vec3));

    pData = GET_ELEMENT_PTR(ConstantSemantics::LIGHT_DIRECTION_VIEWSPACE);
    memcpy(pData, glm::value_ptr(lightDirVS), sizeof(glm::vec3));
  }
//---------------------------------------------------------------------------//
  void Internal::updatePerMaterialData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {

  }
//---------------------------------------------------------------------------//
  void Internal::updatePerObjectData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    ASSERT(_updateStage.pWorldMat);
    ASSERT(_updateStage.pCamera);

    const glm::mat4& viewMat = _updateStage.pCamera->getView();
    const glm::mat4& viewProjMat = _updateStage.pCamera->getViewProjection();
    const glm::mat4& worldMat = *_updateStage.pWorldMat;
    const glm::mat4 worldInvMat(glm::affineInverse(worldMat));
    const glm::mat4 worldView(viewMat * worldMat);
    const glm::mat4 worldViewInv(glm::affineInverse(worldView));
    const glm::mat4 worldViewProj(viewProjMat * worldMat);
    const glm::mat4 worldViewProjInv(glm::inverse(worldViewProj));

    float* pData = GET_ELEMENT_PTR(ConstantSemantics::WORLD_MATRIX);
    memcpy(pData, glm::value_ptr(worldMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::WORLD_INVERSE_MATRIX);
    memcpy(pData, glm::value_ptr(worldInvMat), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::WORLDVIEW_MATRIX);
    memcpy(pData, glm::value_ptr(worldView), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::WORLDVIEW_INVERSE_MATRIX);
    memcpy(pData, glm::value_ptr(worldViewInv), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::WORLDVIEWPROJECTION_MATRIX);
    memcpy(pData, glm::value_ptr(worldViewProj), sizeof(glm::mat4));

    pData = GET_ELEMENT_PTR(ConstantSemantics::WORLDVIEWPROJECTION_INVERSE_MATRIX);
    memcpy(pData, glm::value_ptr(worldViewProjInv), sizeof(glm::mat4));
  }
//---------------------------------------------------------------------------//
#undef BEGIN_CBUFFER_UPDATE
#undef GET_ELEMENT_PTR
#undef END_CBUFFER_UPDATE

#pragma endregion
//---------------------------------------------------------------------------//
  ShaderConstantsManager::ShaderConstantsManager()
  {
    memset(Storage::m_vConstantBuffers, 0x0, sizeof(Storage::m_vConstantBuffers));
    memset(Storage::m_vConstantBufferElements, 0x0, sizeof(Storage::m_vConstantBufferElements));
    memset(Internal::vConstantElementOffsets, -1, sizeof(Internal::vConstantElementOffsets));

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
      if (!Storage::m_vConstantBuffers[(uint32)eType])
        return;
      
      GpuBuffer* pConstantBuffer = Storage::m_vConstantBuffers[(uint32) eType];
      uint8* pConstantData = static_cast<uint8*>(pConstantBuffer->lock(GpuResoruceLockOption::WRITE_PERSISTENT_COHERENT));
      ASSERT(pConstantData);
        
      const Internal::ConstantUpdateFunction& updateFunction = Internal::vConstantUpdateFunctions[(uint32) eType];
      updateFunction(pConstantData, updateStage);

      pConstantBuffer->unlock();
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
                                | (uint32)GpuResourceAccessFlags::DYNAMIC
                                | (uint32)GpuResourceAccessFlags::PERSISTENT_LOCKABLE
                                ;
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
    for (uint32 iConstantBuffer = 0u; iConstantBuffer < (uint32) ConstantBufferType::NUM; ++iConstantBuffer)
    {
      const GpuBuffer* pConstantBuffer = Storage::m_vConstantBuffers[iConstantBuffer];
      _pRenderer->setConstantBuffer(pConstantBuffer, iConstantBuffer);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering