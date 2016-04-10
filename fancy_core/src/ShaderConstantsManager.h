#ifndef INCLUDE_SHADERCONSTANTSUPDATER_H
#define INCLUDE_SHADERCONSTANTSUPDATER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"

// forward decls
namespace Fancy { namespace Rendering {
  struct ConstantBufferElement;
  class Renderer;
  class MaterialPassInstance;
  class RenderContext;
} }

namespace Fancy { namespace Scene {
  class LightComponent;
  class SceneNode;
  class CameraComponent;
} }

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class ConstantBufferType {
    // PER_LAUNCH,
    PER_FRAME,
    PER_VIEWPORT,
    // PER_STAGE,
    PER_CAMERA,
    PER_LIGHT,
    PER_MATERIAL,
    PER_OBJECT,

    NUM,
    NONE
  };
//---------------------------------------------------------------------------//
  struct ShaderConstantsUpdateStage
  {
    ShaderConstantsUpdateStage() 
    : myRenderContext(nullptr)
    , pWorldMat(nullptr)
    , pCamera(nullptr)
    , pMaterial(nullptr)
    , pLight(nullptr) {}

    const RenderContext* myRenderContext;
    const glm::mat4* pWorldMat;
    const Scene::CameraComponent* pCamera;
    const MaterialPassInstance* pMaterial;
    const Scene::LightComponent* pLight;
  };
//---------------------------------------------------------------------------//
  class ShaderConstantsManager
  {
    public:
      static void Init();
      static void Shutdown();

      /// Updates all constants in the provided constantbuffer-type
      static void update(ConstantBufferType eType);
      /// Returns false if no constant buffer exists for this type
      static bool hasBackingBuffer(ConstantBufferType eType);
      /// Registers an element obtained from shader-reflection
      static void CreateBufferWithSize(ConstantBufferType _eConstantBufferType, uint32 _requiredSizeBytes);

      static GpuBuffer* GetConstantBuffer(ConstantBufferType _eConstantBufferType);

      static ConstantBufferType getConstantBufferTypeFromName(const ObjectName& clName);

      static ShaderConstantsUpdateStage updateStage;
    private:
      ShaderConstantsManager();
      ~ShaderConstantsManager();
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_SHADERCONSTANTSUPDATER_H