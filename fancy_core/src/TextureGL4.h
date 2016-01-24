#ifndef INCLUDE_TEXTUREGL4_H
#define INCLUDE_TEXTUREGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "LoadableObject.h"
#include "AdapterGL4.h"
#include "TextureDesc.h"

namespace Fancy { namespace Rendering { namespace GL4 {

class TextureGL4 : public LoadableObject {
public:
//---------------------------------------------------------------------------//
  enum class CreationMethod {
    UPLOADED_ONCE = 0,
    UPLOADED_OFTEN
  };
//---------------------------------------------------------------------------//
  TextureGL4();
  virtual ~TextureGL4();
  bool operator==(const TextureDesc& aDesc) const;

  TextureDesc GetDescription() const;
  void SetFromDescription(const TextureDesc& aDesc);

  virtual bool _init() override;
  virtual bool _destroy() override;

  void create(const TextureCreationParams& clDeclaration, CreationMethod eCreationMethod = CreationMethod::UPLOADED_ONCE);
  void setPixelData(void* pData, uint uDataSizeBytes, 
   glm::u32vec3 rectPosOffset = glm::u32vec3(0,0,0), glm::u32vec3 rectDimensions = glm::u32vec3(0,0,0));
  void* lock(GpuResoruceLockOption option = GpuResoruceLockOption::WRITE_DISCARD); 
  void unlock();

  GLuint getGLhandle() const { return m_uGLhandle; }
  bool isDepthStencilTexture() const { return m_clParameters.bIsDepthStencil; }
  bool isSRGBtexture() const { return m_clStateInfo.isSRGB; }
  bool isLocked() const { return m_clStateInfo.isLocked; }
  bool isCubemap() const {return m_clParameters.eTextureTypeGL == GL_TEXTURE_CUBE_MAP; }
  bool isArrayTexture() const {return m_clStateInfo.isArrayTexture; }
  bool isValid() const { return m_uGLhandle != GLUINT_HANDLE_INVALID; }
  /// retrieve the number of dimensions (1D, 2D, 3D (cubemap & volume))
  uint getNumDimensions() const {return m_clStateInfo.numDimensions; }
  GLenum getInternalFormatGL() const {return m_clParameters.eInternalFormatGL;}
  GLenum getTextureTypeGL() const {return m_clParameters.eTextureTypeGL;}
  const TextureCreationParams& getParameters() const {return m_clParameters;}
  const String& getPath() const { return m_clParameters.path; }
  
  protected:
  //---------------------------------------------------------------------------//
  struct TextureParametersGL : public TextureCreationParams {
    TextureParametersGL() : eFormatGL(0), eInternalFormatGL(0), 
      ePixelTypeGL(0), eTextureTypeGL(0), eTexBindQueryGL(0) {}

    GLenum      eFormatGL;
    GLenum      eInternalFormatGL;
    GLenum      ePixelTypeGL;
    GLenum      eTextureTypeGL;
    GLenum      eTexBindQueryGL;
  };
//---------------------------------------------------------------------------//
  struct TextureInfos {
    TextureInfos() : isSRGB(0), isLocked(0), isArrayTexture(0), isCubemap(0), 
      cachesTextureData(0), numDimensions(0) {}

    uint isSRGB : 1;
    uint isLocked : 1;
    uint isArrayTexture : 1;
    uint isCubemap : 1;
    uint cachesTextureData : 1;
    uint numDimensions : 4;
  };
//---------------------------------------------------------------------------//
  TextureParametersGL   m_clParameters;
  TextureInfos          m_clStateInfo;
  GLuint                m_uGLhandle;
//---------------------------------------------------------------------------//
};

} } } // end of namespace Fancy::Rendering::GL4

#endif

#endif  // INCLUDE_TEXTUREGL4_H