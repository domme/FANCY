#ifndef INCLUDE_TEXTUREGL4_H
#define INCLUDE_TEXTUREGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "LoadableObject.h"

namespace FANCY { namespace Core { namespace Rendering { namespace GL4 {

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

  virtual bool _init() override;
  virtual bool _destroy() override;

  void create(const TextureParameters& clDeclaration, CreationMethod eCreationMethod = CreationMethod::UPLOADED_ONCE);
  void setPixelData(void* pData, uint uDataSizeBytes, 
   glm::u32vec3 rectPosOffset = glm::u32vec3(0,0,0), glm::u32vec3 rectDimensions = glm::u32vec3(0,0,0));
  void* lock(GpuResoruceLockOption option = GpuResoruceLockOption::WRITE_DISCARD); 
  void unlock();

  GLuint getGLhandle() { return m_uGLhandle; }
  bool isDepthStencilTexture() { return m_clParameters.bIsDepthStencil; }
  bool isSRGBtexture() { return m_clStateInfo.isSRGB; }
  bool isLocked() { return m_clStateInfo.isLocked; }
  bool isCubemap() {return m_clParameters.eTextureTypeGL == GL_TEXTURE_CUBE_MAP; }
  bool isArrayTexture() {return m_clStateInfo.isArrayTexture; }
  bool isValid() { return m_uGLhandle != GLUINT_HANDLE_INVALID; }
  /// retrieve the number of dimensions (1D, 2D, 3D (cubemap & volume))
  uint getNumDimensions() {return m_clStateInfo.numDimensions; }

protected:
  //---------------------------------------------------------------------------//
  struct TextureParametersGL : public TextureParameters {
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

} } } } // end of namespace FANCY::Core::Rendering::GL4


#endif  // INCLUDE_TEXTUREGL4_H