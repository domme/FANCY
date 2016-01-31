
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
  struct CBuffer_PER_VIEWPORT
  {
    glm::vec4 c_RenderTargetSize;
  };

  struct CBuffer_PER_FRAME
  {
    glm::vec4 c_TimeParameters;
  };

  struct CBuffer_PER_CAMERA
  {
    glm::mat4 c_ViewMatrix;
    glm::mat4 c_ViewInverseMatrix;
    glm::mat4 c_ProjectionMatrix;
    glm::mat4 c_ProjectionInverseMatrix;
    glm::mat4 c_ViewProjectionMatrix;
    glm::mat4 c_ViewProjectionInverseMatrix;
    glm::vec4 c_NearFarParameters;
    glm::vec4 c_CameraPosWS;
  };

  struct CBuffer_PER_LIGHT
  {
    glm::vec4 c_LightParameters;
    glm::vec4 c_PointSpotParameters;
    glm::vec3 c_LightPosWS;
    glm::vec3 c_LightPosVS;
    glm::vec3 c_LightDirWS;
    glm::vec3 c_LightDirVS;
  };
  
  struct CBuffer_PER_MATERIAL
  {
    glm::vec4 c_MatDiffIntensity;
    glm::vec4 c_MatSpecIntensity;
  };

  struct CBuffer_PER_OBJECT
  {
    glm::mat4 c_WorldMatrix;
    glm::mat4 c_WorldInverseMatrix;
    glm::mat4 c_WorldViewMatrix;
    glm::mat4 c_WorldViewInverseMatrix;
    glm::mat4 c_WorldViewProjectionMatrix;
    glm::mat4 c_WorldViewProjectionInverseMatrix;
  };
  
    typedef std::unordered_map<uint32, ConstantBufferType> ConstantBufferTypeMap;
    ConstantBufferTypeMap locMapConstantBufferTypes;
    /// Gpu-resident buffers representing the datastores of the constant buffers
    GpuBuffer* locConstantBuffers[(uint32)ConstantBufferType::NUM];
//---------------------------------------------------------------------------//
  void locUpdatePerViewportData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    ASSERT(_updateStage.pRenderer);
    const glm::uvec4& uvViewportParams = _updateStage.pRenderer->getViewport();

    CBuffer_PER_VIEWPORT cBuffer;
    cBuffer.c_RenderTargetSize =
      glm::vec4(uvViewportParams.z, uvViewportParams.w, 1.0f / uvViewportParams.z, 1.0f / uvViewportParams.w);

    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void locUpdatePerFrameData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    float* pData = reinterpret_cast<float*>(_pData + offsetof(CBuffer_PER_FRAME, c_TimeParameters));

    CBuffer_PER_FRAME cBuffer;

    cBuffer.c_TimeParameters[0] = Time::getDeltaTime();
    cBuffer.c_TimeParameters[1] = Time::getElapsedTime();
    cBuffer.c_TimeParameters[2] = 0.0f;  // unused
    cBuffer.c_TimeParameters[3] = 0.0f;  // unused

    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void locUpdatePerStageData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {

  }
//---------------------------------------------------------------------------//
  void locUpdatePerCameraData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
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

    CBuffer_PER_CAMERA cBuffer;
    cBuffer.c_ViewMatrix = viewMat;
    cBuffer.c_ViewInverseMatrix = viewInvMat;
    cBuffer.c_ProjectionMatrix = projMat;
    cBuffer.c_ProjectionInverseMatrix = projInvMat;
    cBuffer.c_ViewProjectionMatrix = viewProjMat;
    cBuffer.c_ViewProjectionInverseMatrix = viewProjInv;
    cBuffer.c_NearFarParameters[0] = fNear;
    cBuffer.c_NearFarParameters[1] = fFar;
    cBuffer.c_NearFarParameters[2] = fNear / fFar;
    cBuffer.c_NearFarParameters[3] = 1.0f / fFar;
    cBuffer.c_CameraPosWS = glm::column(viewInvMat, 3);

    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void locUpdatePerLightData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
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
    
    CBuffer_PER_LIGHT cBuffer;
    cBuffer.c_LightParameters = lightParams;
    cBuffer.c_PointSpotParameters = pointSpotParams;
    cBuffer.c_LightPosWS = lightTransform.getPosition();
    cBuffer.c_LightPosVS = lightPosVS;
    cBuffer.c_LightDirWS = lightTransform.forward();
    cBuffer.c_LightDirVS = lightDirVS;
    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void locUpdatePerMaterialData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
  {
    CBuffer_PER_MATERIAL cBuffer;
    memset(&cBuffer, 0u, sizeof(cBuffer));
    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  void locUpdatePerObjectData( uint8* _pData, const ShaderConstantsUpdateStage& _updateStage )
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

    CBuffer_PER_OBJECT cBuffer;
    cBuffer.c_WorldMatrix = worldMat;
    cBuffer.c_WorldInverseMatrix = worldInvMat;
    cBuffer.c_WorldViewMatrix = worldView;
    cBuffer.c_WorldViewInverseMatrix = worldViewInv;
    cBuffer.c_WorldViewProjectionMatrix = worldViewProj;
    cBuffer.c_WorldViewProjectionInverseMatrix = worldViewProjInv;
    memcpy(_pData, &cBuffer, sizeof(cBuffer));
  }
//---------------------------------------------------------------------------//
  ShaderConstantsManager::ShaderConstantsManager()
  {
    
  }
//---------------------------------------------------------------------------//
  ShaderConstantsManager::~ShaderConstantsManager()
  {
    
  }  
//---------------------------------------------------------------------------//
  bool ShaderConstantsManager::hasBackingBuffer(ConstantBufferType eType)
  {
    return locConstantBuffers[(uint32)eType] != nullptr;
  }
//---------------------------------------------------------------------------//
  Fancy::Rendering::ConstantBufferType ShaderConstantsManager::getConstantBufferTypeFromName( const ObjectName& clName )
  {
    ConstantBufferTypeMap::const_iterator it = locMapConstantBufferTypes.find(clName);
    ASSERT(it != locMapConstantBufferTypes.end());
    if (it != locMapConstantBufferTypes.end())
    {
      return (*it).second;
    }

    return ConstantBufferType::NONE;
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::Init()
  {
    memset(locConstantBuffers, 0x0, sizeof(locConstantBuffers));

    CreateBufferWithSize(ConstantBufferType::PER_CAMERA, sizeof(CBuffer_PER_CAMERA));
    CreateBufferWithSize(ConstantBufferType::PER_VIEWPORT, sizeof(CBuffer_PER_VIEWPORT));
    CreateBufferWithSize(ConstantBufferType::PER_FRAME, sizeof(CBuffer_PER_FRAME));
    CreateBufferWithSize(ConstantBufferType::PER_LIGHT, sizeof(CBuffer_PER_LIGHT));
    CreateBufferWithSize(ConstantBufferType::PER_MATERIAL, sizeof(CBuffer_PER_MATERIAL));
    CreateBufferWithSize(ConstantBufferType::PER_OBJECT, sizeof(CBuffer_PER_OBJECT));
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::Shutdown()
  {
    for (uint32 i = 0; i < _countof(locConstantBuffers); ++i)
    {
      if (locConstantBuffers[i] != nullptr)
      {
        FANCY_DELETE(locConstantBuffers[i], MemoryCategory::BUFFERS);
        locConstantBuffers[i] = nullptr;
      }
    }
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::update( ConstantBufferType eType )
  {
      if (nullptr == locConstantBuffers[(uint32)eType])
        return;
      
      GpuBuffer* pConstantBuffer = locConstantBuffers[(uint32) eType];
      uint8* pConstantData = static_cast<uint8*>(pConstantBuffer->lock(GpuResoruceLockOption::WRITE_PERSISTENT_COHERENT));
      ASSERT(pConstantData);

      switch(eType)
      {
        case ConstantBufferType::PER_FRAME: 
          locUpdatePerFrameData(pConstantData, updateStage);
          break;
        case ConstantBufferType::PER_VIEWPORT: 
          locUpdatePerViewportData(pConstantData, updateStage);
          break;
        case ConstantBufferType::PER_CAMERA: 
          locUpdatePerCameraData(pConstantData, updateStage);
          break;
        case ConstantBufferType::PER_LIGHT: 
          locUpdatePerLightData(pConstantData, updateStage);
          break;
        case ConstantBufferType::PER_MATERIAL: 
          locUpdatePerMaterialData(pConstantData, updateStage);
          break;
        case ConstantBufferType::PER_OBJECT: 
          locUpdatePerObjectData(pConstantData, updateStage);
          break;
        case ConstantBufferType::NUM: break;
        case ConstantBufferType::NONE: break;
        default: break;
      }

      pConstantBuffer->unlock();
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::CreateBufferWithSize(ConstantBufferType _eConstantBufferType, uint32 _requiredSizeBytes)
  {
    // Allocate the constant buffer storage if needed
    const uint32 uBufferTypeIdx = static_cast<uint32>(_eConstantBufferType);
    if (nullptr == locConstantBuffers[uBufferTypeIdx])
    {
      GpuBuffer* const pBuffer = FANCY_NEW(GpuBuffer, MemoryCategory::BUFFERS);

      GpuBufferCreationParams bufferParams;
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

      locConstantBuffers[uBufferTypeIdx] = pBuffer;
    }
    
    //ASSERT_M(locConstantBuffers[uBufferTypeIdx]->getTotalSizeBytes() == _requiredSizeBytes, "Requested the same constant buffer with two different sizes");
  }
//---------------------------------------------------------------------------//
  void ShaderConstantsManager::bindBuffers(Rendering::Renderer* _pRenderer)
  {
    for (uint32 iConstantBuffer = 0u; iConstantBuffer < (uint32) ConstantBufferType::NUM; ++iConstantBuffer)
    {
      const GpuBuffer* pConstantBuffer = locConstantBuffers[iConstantBuffer];
      _pRenderer->setConstantBuffer(pConstantBuffer, iConstantBuffer);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering