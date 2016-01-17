#include "TextureGL4.h"

#if defined (RENDERER_OPENGL4)

namespace Fancy { namespace Rendering {  namespace GL4 {
//---------------------------------------------------------------------------//
  TextureGL4::TextureGL4() :
    m_uGLhandle(GLUINT_HANDLE_INVALID)
  {
    
  }
//---------------------------------------------------------------------------//
  TextureGL4::~TextureGL4()
  {
    // Destroy GL-resource
    destroy();

    // If necessary, destroy CPU-resource
    if(m_clStateInfo.cachesTextureData) {
      ASSERT_M(m_clParameters.pPixelData, "Invalid pixel data");
      FANCY_FREE(m_clParameters.pPixelData, MemoryCategory::TEXTURES);
    }

    // reset internal values
    m_clParameters = TextureParametersGL();
    m_clStateInfo = TextureInfos();
    m_uGLhandle = GLUINT_HANDLE_INVALID;
  }
//---------------------------------------------------------------------------//
  bool TextureGL4::operator==(const TextureDesc& aDesc) const 
  {

  }
//---------------------------------------------------------------------------//
  TextureDesc TextureGL4::GetDescription() const
  {
    TextureDesc desc;

    

    return desc;
  }
//---------------------------------------------------------------------------//
  bool TextureGL4::_init()
  {
    create(m_clParameters, m_clStateInfo.cachesTextureData ? 
      CreationMethod::UPLOADED_OFTEN : CreationMethod::UPLOADED_ONCE);

    return isValid();
  }
//---------------------------------------------------------------------------//
  bool TextureGL4::_destroy()
  {
    // Only destroy the GL-resource here to enable re-initialization
    if(isValid()) {
      glDeleteTextures(1, &m_uGLhandle);
      m_uGLhandle = GLUINT_HANDLE_INVALID;
    }

    return !isValid();
  }
//---------------------------------------------------------------------------//
  void TextureGL4::create(const TextureCreationParams& clDeclaration, CreationMethod eCreationMethod /*= CreationMethod::UPLOADED_ONCE*/)
  {
    // TODO: Implement Cubemap- and array textures 
    destroy();

    TextureCreationParams* pBaseParams = &m_clParameters;
    *pBaseParams = clDeclaration;

    GLenum eGLformat, eGLinternalFormat, eGLpixelType;
    Adapter::mapGLpixelFormats(clDeclaration.eFormat, clDeclaration.bIsDepthStencil, 
      eGLformat, eGLinternalFormat, eGLpixelType);

    m_clParameters.eFormatGL = eGLformat;
    m_clParameters.eInternalFormatGL = eGLinternalFormat;
    m_clParameters.ePixelTypeGL = eGLpixelType;
    
    GLenum eTextureType = 0u;
    GLenum eTexBindingQuery = 0u;
    GLuint uNumDimensions = 0u;

    ASSERT_M(clDeclaration.u16Width > 0u, "Invalid texture dimensions");
    if (clDeclaration.u16Height == 0u && clDeclaration.u16Depth == 0u )
    {
      eTextureType = GL_TEXTURE_1D;
      eTexBindingQuery = GL_TEXTURE_BINDING_1D;
      uNumDimensions = 1u;
    } 
    else if (clDeclaration.u16Height > 0u && clDeclaration.u16Depth == 0u) 
    {
      eTextureType = GL_TEXTURE_2D;
      eTexBindingQuery = GL_TEXTURE_BINDING_2D;
      uNumDimensions = 2u;
    }
    else if (clDeclaration.u16Height > 0u && clDeclaration.u16Depth > 0u)
    {
      eTextureType = GL_TEXTURE_3D;
      eTexBindingQuery = GL_TEXTURE_BINDING_3D;
      uNumDimensions = 3u;
    }
    
    ASSERT_M (uNumDimensions > 0u, "Invalid texture dimensions");
    m_clParameters.eTextureTypeGL = eTextureType;
    m_clParameters.eTexBindQueryGL = eTexBindingQuery;

    m_clStateInfo.isArrayTexture = false; // TODO: Implement
    m_clStateInfo.isCubemap = false; // TODO: Implement
    m_clStateInfo.isLocked = false;
    m_clStateInfo.isSRGB = (eGLinternalFormat == GL_SRGB8_ALPHA8 || eGLinternalFormat == GL_SRGB8);
    m_clStateInfo.numDimensions = uNumDimensions;

    // Determine the max number of miplevels
    if (m_clParameters.u8NumMipLevels > 0u)
    {
      uint8 u8MipLevelsWidth  = clDeclaration.u16Width == 0u ? 0u : glm::log2(clDeclaration.u16Width);
      uint8 u8MipLevelsHeight = clDeclaration.u16Height == 0u ? 0u : glm::log2(clDeclaration.u16Height);
      uint8 u8MipLevelsDepth  = clDeclaration.u16Depth == 0u ? 0u : glm::log2(clDeclaration.u16Depth);

      uint8 u8MaxMipLevels = u8MipLevelsWidth;
      if (uNumDimensions > 1u) {
        u8MaxMipLevels = glm::min(u8MaxMipLevels, u8MipLevelsHeight);
      }
      if (uNumDimensions > 2u) {
        u8MaxMipLevels = glm::min(u8MaxMipLevels, u8MipLevelsDepth);
      }

      m_clParameters.u8NumMipLevels = 
        glm::min(m_clParameters.u8NumMipLevels, u8MaxMipLevels);
    }

    // Internally store the raw texture data if required
    m_clStateInfo.cachesTextureData = false;
    if (m_clParameters.pPixelData != nullptr && eCreationMethod == CreationMethod::UPLOADED_OFTEN) {
      void* pDataCache = FANCY_ALLOCATE(m_clParameters.uPixelDataSizeBytes, MemoryCategory::TEXTURES);
      memcpy(pDataCache, m_clParameters.pPixelData, m_clParameters.uPixelDataSizeBytes);
      m_clParameters.pPixelData = pDataCache;
      m_clStateInfo.cachesTextureData = true;
    }

    // determine the currently bound texture
    GLint origBoundTexture;
    glGetIntegerv(eTexBindingQuery, &origBoundTexture);
  
    // construct the new texture
    glGenTextures(1u, &m_uGLhandle);
    glBindTexture(eTextureType, m_uGLhandle);

    if (uNumDimensions == 2u) {
      glTexStorage2D(eTextureType, m_clParameters.u8NumMipLevels, eGLinternalFormat, 
        m_clParameters.u16Width, m_clParameters.u16Height );
    }
    else if (uNumDimensions == 3u) {
      glTexStorage3D(eTextureType, m_clParameters.u8NumMipLevels, eGLinternalFormat, 
        m_clParameters.u16Width, m_clParameters.u16Height, m_clParameters.u16Depth );
    }
    else {
      glTexStorage1D(eTextureType, m_clParameters.u8NumMipLevels, eGLinternalFormat, 
        m_clParameters.u16Width );
    }

    if (m_clParameters.pPixelData != nullptr) {
      setPixelData(m_clParameters.pPixelData, m_clParameters.uPixelDataSizeBytes);
    }

    // restore originally bound texture
    glBindTexture(eTextureType, static_cast<GLuint>(origBoundTexture));
  }
//---------------------------------------------------------------------------//
  void TextureGL4::setPixelData( void* pData, uint uDataSizeBytes, 
    glm::u32vec3 rectPosOffset /* = glm::ivec3(0,0,0) */, glm::u32vec3 rectDimensions /*= glm::ivec3(0,0,0) */ )
  {
    ASSERT_M(pData && uDataSizeBytes > 0, "Invalid texture data specified");
    ASSERT_M(isValid(), "OpenGL-texture is not valid");

    const uint32 uNumDimensions = m_clStateInfo.numDimensions;
    const bool bUsesTextureCache = m_clStateInfo.cachesTextureData;
    const bool bSetSubregion = rectDimensions.x > 0 || rectDimensions.y > 0 || rectDimensions.z > 0;

    // Free and recreate the texture cache
    if (bUsesTextureCache && pData != m_clParameters.pPixelData) {
      ASSERT_M( !bSetSubregion, "Sub-memcpys in subregions of cached pixeldata not implemented yet" );
      ASSERT_M(m_clParameters.uPixelDataSizeBytes == uDataSizeBytes, "Data size does not match");
      memcpy(m_clParameters.pPixelData, pData, uDataSizeBytes);
    }

    GLint origBoundTexture;
    glGetIntegerv(m_clParameters.eTexBindQueryGL, &origBoundTexture);
    const bool bRebindTexture = origBoundTexture != m_uGLhandle;

    if (bRebindTexture) {
      glBindTexture(m_clParameters.eTextureTypeGL, m_uGLhandle);
    }

    uint32 offsetX = 0;
    uint32 offsetY = 0;
    uint32 offsetZ = 0;
    uint32 width = m_clParameters.u16Width;
    uint32 height = m_clParameters.u16Height;
    uint32 depth = m_clParameters.u16Depth;
    
    if (bSetSubregion) {
      offsetX = rectPosOffset.x;
      offsetY = rectPosOffset.y;
      offsetZ = rectPosOffset.z;

      width = glm::min(width, rectDimensions.x);
      height = glm::min(height, rectDimensions.y);
      depth = glm::min(depth, rectDimensions.z);
    }
    
    
    // Upload the pixels to the GPU
    if (uNumDimensions == 2) {
      glTexSubImage2D(m_clParameters.eTextureTypeGL, 0, 
        offsetX, offsetY, width, height,
        m_clParameters.eFormatGL, m_clParameters.ePixelTypeGL, pData );
    } else if(uNumDimensions == 3) {
      glTexSubImage3D(m_clParameters.eTextureTypeGL, 0, 
        offsetX, offsetY, offsetZ, width, height, depth, 
        m_clParameters.eFormatGL, m_clParameters.ePixelTypeGL, pData );
    } else {
      glTexSubImage1D(m_clParameters.eTextureTypeGL, 0, 
        offsetX, width, 
        m_clParameters.eFormatGL, m_clParameters.ePixelTypeGL, pData );
    }

    /// TODO: Let the user provide the data for mipmap-levels
    if (m_clParameters.u8NumMipLevels > 0) {
      glGenerateMipmap(m_clParameters.eTextureTypeGL);
    }
    
    if (bRebindTexture) {
      glBindTexture(m_clParameters.eTextureTypeGL, origBoundTexture);
    }
  }
//---------------------------------------------------------------------------//
  void* TextureGL4::lock( GpuResoruceLockOption option /*= GpuResoruceMapOption::WRITE_DISCARD*/ )
  {
    ASSERT_M(false, "lock/unlock not supported on OpenGL textures");
    return nullptr;
  }
//---------------------------------------------------------------------------//
  void TextureGL4::unlock()
  {
    ASSERT_M(false, "lock/unlock not supported on OpenGL textures");
  }
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering::GL4

#endif
