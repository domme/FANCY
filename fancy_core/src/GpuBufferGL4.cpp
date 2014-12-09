#include "GpuBufferGL4.h"
#include "RendererPrerequisites.h"

#include "TimeManager.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  namespace internal 
  {
    GLenum mapBufferUsage(GpuBufferUsage eGeneralUsage, GLenum& eBindingQueryGL);
    GLuint mapLockOption(Rendering::GpuResoruceLockOption eLockOption);
  }
//---------------------------------------------------------------------------//
  GLenum internal::mapBufferUsage(GpuBufferUsage eGeneralUsage, GLenum& eBindingQueryGL) 
  {
    GLenum eUsageGL;

    switch (eGeneralUsage)
    {
      case GpuBufferUsage::CONSTANT_BUFFER: 
        eUsageGL = GL_UNIFORM_BUFFER; 
        eBindingQueryGL = GL_UNIFORM_BUFFER_BINDING;
        break;
      case GpuBufferUsage::VERTEX_BUFFER:
        eUsageGL = GL_ARRAY_BUFFER;
        eBindingQueryGL = GL_ARRAY_BUFFER_BINDING;
        break;
      case GpuBufferUsage::INDEX_BUFFER:
        eUsageGL = GL_ELEMENT_ARRAY_BUFFER;
        eBindingQueryGL = GL_ELEMENT_ARRAY_BUFFER_BINDING;
        break;
      case GpuBufferUsage::DRAW_INDIRECT_BUFFER:
        eUsageGL = GL_DRAW_INDIRECT_BUFFER;
        eBindingQueryGL = GL_DRAW_INDIRECT_BUFFER_BINDING;
        break;
      case GpuBufferUsage::DISPATCH_INDIRECT_BUFFER:
        eUsageGL = GL_DISPATCH_INDIRECT_BUFFER;
        eBindingQueryGL = GL_DISPATCH_INDIRECT_BUFFER_BINDING;
        break;
      case GpuBufferUsage::RESOURCE_BUFFER:
        eUsageGL = GL_TEXTURE_BUFFER;
        eBindingQueryGL = GL_TEXTURE_BINDING_BUFFER; // WTF? Oo
        break;
      case GpuBufferUsage::RESOURCE_BUFFER_RW:
        // In GL, we don't need to distinguish between read-only and read/write at this point
        eUsageGL = GL_TEXTURE_BUFFER;  
        eBindingQueryGL = GL_TEXTURE_BINDING_BUFFER;
        break;
      case GpuBufferUsage::RESOURCE_BUFFER_LARGE:
        eUsageGL = GL_SHADER_STORAGE_BUFFER;
        eBindingQueryGL = GL_SHADER_STORAGE_BUFFER_BINDING;
        break;
      case GpuBufferUsage::RESOURCE_BUFFER_LARGE_RW:
        // Again, no need to distinguish here
        eUsageGL = GL_SHADER_STORAGE_BUFFER;
        eBindingQueryGL = GL_SHADER_STORAGE_BUFFER_BINDING;
        break;
      default:
        ASSERT_M(false, "Missing GL implementation");
        break;
    }

    return eUsageGL;
  }
//---------------------------------------------------------------------------//
  GLuint internal::mapLockOption(Rendering::GpuResoruceLockOption eLockOption)
  {
    switch (eLockOption)
    {
    case GpuResoruceLockOption::READ: 
      return GL_MAP_READ_BIT;

    case GpuResoruceLockOption::WRITE: 
      return GL_MAP_WRITE_BIT;

    case GpuResoruceLockOption::READ_WRITE: 
      return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;

    case GpuResoruceLockOption::WRITE_DISCARD: 
      return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;

    case GpuResoruceLockOption::READ_WRITE_DISCARD: 
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;

    case GpuResoruceLockOption::READ_UNSYNCHRONIZED:
      return GL_MAP_READ_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    case GpuResoruceLockOption::WRITE_UNSYNCHRONIZED:
      return GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    case GpuResoruceLockOption::READ_WRITE_UNSYNCHRONIZED:
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    case GpuResoruceLockOption::READ_PERSISTENT:
      return GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT;

    case GpuResoruceLockOption::WRITE_PERSISTENT:
      return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

    case GpuResoruceLockOption::READ_WRITE_PERSISTENT:
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

    case GpuResoruceLockOption::READ_PERSISTENT_COHERENT:
      return GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    case GpuResoruceLockOption::WRITE_PERSISTENT_COHERENT:
      return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |GL_MAP_COHERENT_BIT;

    case GpuResoruceLockOption::READ_WRITE_PERSISTENT_COHERENT:
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    default: ASSERT_M(false, "Missing GL implementation");
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
    }
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  GpuBufferGL4::GpuBufferGL4() :
    m_pCachedLockPtr(nullptr),
    m_uDerivedInternalBufferCount(1u),
    m_eMultiBufferStrategy(MultiBufferingStrategy::NONE)
  {

  }
//---------------------------------------------------------------------------//
  GpuBufferGL4::~GpuBufferGL4()
  {
    destroy();
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::destroy()
  {
    if (!isValid()) {
      return;
    }

    ASSERT_M( isLockedPersistent() || !isLocked(), "Trying to destroy a locked buffer");

    // Persistent buffers stay locked and have to be unlocked here
    if (isLockedPersistent())
    {
      for (uint32 i = 0; i < MultiBuffering::kGpuMultiBufferingCount; ++i)
      {
        _unlock(i);
      }
    }

    for (uint32 i = 0; i < MultiBuffering::kGpuMultiBufferingCount; ++i)
    {
      glDeleteBuffers(1, &m_vGLhandles[i]);
    }
    m_vGLhandles.clear();

    m_clStateInfos.isLocked = false;
    m_clStateInfos.isLockedPersistent = false;
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::create( const GpuBufferParameters& clParameters, void* pInitialData /*= nullptr*/ )
  {
    destroy();

    ASSERT_M(clParameters.uElementSizeBytes > 0 && clParameters.uNumElements > 0,
       "Invalid buffer size specified");

    static_cast<GpuBufferParameters>(m_clParameters) = clParameters;

    GLenum eUsageTarget;
    GLenum eBindingQuery;
    eUsageTarget = internal::mapBufferUsage(clParameters.ePrimaryUsageType, eBindingQuery);
    m_clParameters.eInitialBufferTargetGL = eUsageTarget;
    m_clParameters.eBindingQueryType = eBindingQuery;

    uint32 uRequestedAccessFlags = clParameters.uAccessFlags;
    uint32 uAccessFlagsGL = 0;

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::READ) > 0) {
      uAccessFlagsGL |= GL_MAP_READ_BIT;
    }

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::WRITE) > 0) {
      uAccessFlagsGL |= GL_MAP_WRITE_BIT;
    }

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::DYNAMIC) > 0) {
      uAccessFlagsGL |= GL_DYNAMIC_STORAGE_BIT;
    }

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::COHERENT) > 0) {
      uAccessFlagsGL |= GL_MAP_COHERENT_BIT;
    }

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0) {
      uAccessFlagsGL |= GL_CLIENT_STORAGE_BIT;
    }

    if ((uRequestedAccessFlags & (uint) GpuResourceAccessFlags::PERSISTENT_LOCKABLE) > 0) {
      uAccessFlagsGL |= GL_MAP_PERSISTENT_BIT;
    }

    // Sanity-checks on the access flags (following the OpenGL 4.4 standard)
    if((uAccessFlagsGL & GL_MAP_PERSISTENT_BIT) > 0 &&
         ((uAccessFlagsGL & GL_MAP_WRITE_BIT) == 0 &&
          uAccessFlagsGL & GL_MAP_READ_BIT) == 0) 
    {
      ASSERT_M(false, "A persistent buffer without CPU-access doesn't make sense");
    }

    if ((uAccessFlagsGL & GL_MAP_COHERENT_BIT) > 0 &&
        (uAccessFlagsGL & GL_MAP_PERSISTENT_BIT) == 0) 
    {
      ASSERT_M(false, "Coherent buffers also need to be persistent");
    }

    // Derive the appropriate multibuffering settings
    if (!m_clParameters.bIsMultiBuffered)
    {
      m_uDerivedInternalBufferCount = 1u;
      m_eMultiBufferStrategy = MultiBufferingStrategy::NONE;
    }
    else  // Multibuffering is used 
    {
      // Note: Here we assume that persistant mapping also means the buffer is
      //        multibuffered internally using an offset... have to see if this is always desired
      if ( (uAccessFlagsGL & GL_MAP_PERSISTENT_BIT) > 0 ) 
      {
        m_eMultiBufferStrategy = MultiBufferingStrategy::OFFSETS;
        m_uDerivedInternalBufferCount = 1u;
      } 
      else 
      {
        m_eMultiBufferStrategy = MultiBufferingStrategy::BUFFERS;
        m_uDerivedInternalBufferCount = MultiBuffering::kGpuMultiBufferingCount;
      }
    }

    // Create and allocate the buffer
    m_clParameters.uAccessFlagsGL = uAccessFlagsGL;
    m_clParameters.uTotalSizeBytes = 
      clParameters.uNumElements * clParameters.uElementSizeBytes;

    // Retrieve the currently bound buffer to restore it later
    GLint originalBufferBinding;
    glGetIntegerv(eBindingQuery, &originalBufferBinding);

    if (m_eMultiBufferStrategy == MultiBufferingStrategy::NONE || 
        m_eMultiBufferStrategy == MultiBufferingStrategy::BUFFERS)
    {
      for (uint32 i = 0; i < m_uDerivedInternalBufferCount; ++i)
      {
        uint32 uGlHandle;

        glGenBuffers(1, &uGlHandle);
        glBindBuffer(eUsageTarget, uGlHandle);

        glBufferStorage(eUsageTarget, 
            m_clParameters.uTotalSizeBytes, 
            pInitialData, uAccessFlagsGL);

        m_vGLhandles.push_back(uGlHandle);
      }
    }
    else  // MultiBufferingStrategy::OFFSETS
    {
      uint32 uGlHandle;

      glGenBuffers(1, &uGlHandle);
      glBindBuffer(eUsageTarget, uGlHandle);

      // Allocate additional storage but update the initial data in substeps 
      // (because multibuffering should be transparent for high-level and pInitialData is
      // assumed to point to a single-buffer data store)
      glBufferStorage(eUsageTarget, 
        m_clParameters.uTotalSizeBytes * MultiBuffering::kGpuMultiBufferingCount, 
        nullptr, uAccessFlagsGL);

      for (uint32 iBufferPart = 0; iBufferPart < MultiBuffering::kGpuMultiBufferingCount; ++iBufferPart)
      {
        glBufferSubData(eUsageTarget,
          iBufferPart * m_clParameters.uTotalSizeBytes,
          m_clParameters.uTotalSizeBytes, pInitialData );
      }

      m_vGLhandles.push_back(uGlHandle);
    }

    // Restore the originally bound buffer
    glBindBuffer(eUsageTarget, static_cast<GLuint>(originalBufferBinding));
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::setBufferData( void* pData, uint uOffsetElements /*= 0*/, uint uNumElements /*= 0*/ )
  {
    ASSERT_M(pData != nullptr, "Invalid buffer data");
    ASSERT_M((m_clParameters.uAccessFlagsGL & GL_DYNAMIC_STORAGE_BIT) > 0, "Buffer storage is immuatable");

    GLint origBoundBuffer;
    const GLenum targetGL = m_clParameters.eInitialBufferTargetGL;
    const GLenum bindingQueryGL = m_clParameters.eBindingQueryType;

    glGetIntegerv(bindingQueryGL, &origBoundBuffer);
    glBindBuffer(targetGL, getGLhandle());

    // Try to use orphaning/renaming if the whole buffer is being reset
    const bool bResetWholeBuffer = 
      (uOffsetElements == 0 && uNumElements == 0) ||
      (uOffsetElements == 0 && uNumElements == m_clParameters.uNumElements);

    if(bResetWholeBuffer) {
      uOffsetElements = m_clParameters.uNumElements;
      uOffsetElements = 0;
    }
    
    uint offsetBytes = uOffsetElements * m_clParameters.uElementSizeBytes;
    uint rangeBytes = uNumElements * m_clParameters.uElementSizeBytes;
    glBufferSubData(targetGL, offsetBytes, rangeBytes, pData );

    glBindBuffer(targetGL, origBoundBuffer);
  }
//---------------------------------------------------------------------------//
  void* GpuBufferGL4::lock( GpuResoruceLockOption eLockOption, 
    uint uOffsetElements /* = 0 */, uint uNumElements /* = 0 */ )
  {
    const GLuint uLockOptionsGL = internal::mapLockOption(eLockOption);

    // We always keep persistently locked buffers locked for its entrie lifetime, so just return the
    // cached and appropriately offset pointer here
    if ((uLockOptionsGL & GL_MAP_PERSISTENT_BIT) > 0 && isLocked())
    {
      if (m_eMultiBufferStrategy == MultiBufferingStrategy::NONE ||
          m_eMultiBufferStrategy == MultiBufferingStrategy::BUFFERS)
      {
        return m_pCachedLockPtr;
      }
      else  // MultiBufferingStrategy::OFFSETS
      {
        return static_cast<uint8*>(m_pCachedLockPtr) + (getBufferIndex() * m_clParameters.uTotalSizeBytes); 
      }
    }
    
    ASSERT_M(isValid(), "Tried to lock an uninitialized buffer");
    ASSERT_M(!isLocked(), "Buffer is already locked");
    ASSERT_M(uOffsetElements < m_clParameters.uNumElements, "Invalid lock-range provided");

    const uint offsetBytes = uOffsetElements * m_clParameters.uElementSizeBytes;
    const uint rangeBytes = uNumElements * m_clParameters.uElementSizeBytes;
    
    GLint origBoundBuffer;
    glGetIntegerv(m_clParameters.eBindingQueryType, &origBoundBuffer);
    glBindBuffer(m_clParameters.eInitialBufferTargetGL, getGLhandle());

    m_pCachedLockPtr = glMapBufferRange(m_clParameters.eInitialBufferTargetGL, 
         offsetBytes, rangeBytes, uLockOptionsGL);

    glBindBuffer(m_clParameters.eInitialBufferTargetGL, origBoundBuffer);

    ASSERT_M(m_pCachedLockPtr, "Buffer-lock failed");
    m_clStateInfos.isLocked = true;
    if ((uLockOptionsGL & GL_MAP_PERSISTENT_BIT) > 0u)
    {
      m_clStateInfos.isLockedPersistent = true;
    }

    void* returnPtr = m_pCachedLockPtr;

    if (m_eMultiBufferStrategy == MultiBufferingStrategy::OFFSETS)
    {
      returnPtr = static_cast<uint8*>(returnPtr) + (getBufferIndex() * m_clParameters.uTotalSizeBytes);
    }

    return returnPtr;
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::unlock()
  {
    // Don't unlock persistently mapped resources
    if (isLockedPersistent() && isLocked())
    {
      return;
    }

    ASSERT_M(isLocked(), "Tried to unlock and unlocked buffer");
    _unlock(getBufferIndex());

    m_clStateInfos.isLocked = false;
    m_clStateInfos.isLockedPersistent = false;
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::_unlock(uint32 uBufferIndex)
  {
    GLint origBoundBuffer;
    glGetIntegerv(m_clParameters.eBindingQueryType, &origBoundBuffer);
    glBindBuffer(m_clParameters.eInitialBufferTargetGL, m_vGLhandles[uBufferIndex]);

    glUnmapBuffer(m_clParameters.eInitialBufferTargetGL);

    glBindBuffer(m_clParameters.eInitialBufferTargetGL, origBoundBuffer);
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering::GL4