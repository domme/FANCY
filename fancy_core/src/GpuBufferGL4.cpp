#include "GpuBufferGL4.h"
#include "RendererPrerequisites.h"


namespace FANCY { namespace Core { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  namespace internal 
  {
    GLenum mapBufferUsage(BufferUsage eGeneralUsage, GLenum& eBindingQueryGL);
    GLenum mapLockOption(Rendering::GpuResoruceLockOption eLockOption);
  }
//---------------------------------------------------------------------------//
  GLenum internal::mapBufferUsage(BufferUsage eGeneralUsage, GLenum& eBindingQueryGL) 
  {
    GLenum eUsageGL;

    switch (eGeneralUsage)
    {
      case BufferUsage::CONSTANT_BUFFER: 
        eUsageGL = GL_UNIFORM_BUFFER; 
        eBindingQueryGL = GL_UNIFORM_BUFFER_BINDING;
        break;
      case BufferUsage::VERTEX_BUFFER:
        eUsageGL = GL_ARRAY_BUFFER;
        eBindingQueryGL = GL_ARRAY_BUFFER_BINDING;
        break;
      case BufferUsage::INDEX_BUFFER:
        eUsageGL = GL_ELEMENT_ARRAY_BUFFER;
        eBindingQueryGL = GL_ELEMENT_ARRAY_BUFFER_BINDING;
        break;
      case BufferUsage::DRAW_INDIRECT_BUFFER:
        eUsageGL = GL_DRAW_INDIRECT_BUFFER;
        eBindingQueryGL = GL_DRAW_INDIRECT_BUFFER_BINDING;
        break;
      case BufferUsage::DISPATCH_INDIRECT_BUFFER:
        eUsageGL = GL_DISPATCH_INDIRECT_BUFFER;
        eBindingQueryGL = GL_DISPATCH_INDIRECT_BUFFER_BINDING;
        break;
      case BufferUsage::RESOURCE_BUFFER:
        eUsageGL = GL_TEXTURE_BUFFER;
        eBindingQueryGL = GL_TEXTURE_BINDING_BUFFER; // WTF? Oo
        break;
      case BufferUsage::RESOURCE_BUFFER_RW:
        // In GL, we don't need to distinguish between read-only and read/write at this point
        eUsageGL = GL_TEXTURE_BUFFER;  
        eBindingQueryGL = GL_TEXTURE_BINDING_BUFFER;
        break;
      case BufferUsage::RESOURCE_BUFFER_LARGE:
        eUsageGL = GL_SHADER_STORAGE_BUFFER;
        eBindingQueryGL = GL_SHADER_STORAGE_BUFFER_BINDING;
        break;
      case BufferUsage::RESOURCE_BUFFER_LARGE_RW:
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
  GLenum internal::mapLockOption(Rendering::GpuResoruceLockOption eLockOption)
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
  GpuBufferGL4::GpuBufferGL4() :
    m_uGLhandle(GLUINT_HANDLE_INVALID)
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

    ASSERT_M(!isLocked(), "Trying to destroy a locked buffer");

    glDeleteBuffers(1, &m_uGLhandle);
    m_uGLhandle = GLUINT_HANDLE_INVALID;
  }
//---------------------------------------------------------------------------//
  void GpuBufferGL4::create( const BufferParameters& clParameters, void* pInitialData /*= nullptr*/ )
  {
    destroy();

    ASSERT_M(clParameters.uElementSizeBytes > 0 && clParameters.uNumElements > 0,
       "Invalid buffer size specified");

    static_cast<BufferParameters>(m_clParameters) = clParameters;

    GLenum eUsageTarget;
    GLenum eBindingQuery;
    internal::mapBufferUsage(clParameters.ePrimaryUsageType, eBindingQuery);
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

    // Retrieve the currently bound buffer to restore it later
    GLint originalBufferBinding;
    glGetIntegerv(eBindingQuery, &originalBufferBinding);

    // Create the actual OpenGL-buffer
    glGenBuffers(1, &m_uGLhandle);
    // Here, we use the non-indexed version of bindBuffer for all buffer-targets.
    // Following the standard, this should be valid without modifying the indexed buffer states
    glBindBuffer(eUsageTarget, m_uGLhandle);

    m_clParameters.uAccessFlagsGL = uAccessFlagsGL;
    m_clParameters.uTotalSizeBytes = clParameters.uNumElements * clParameters.uElementSizeBytes;
    glBufferStorage(eUsageTarget, m_clParameters.uTotalSizeBytes, pInitialData, uAccessFlagsGL);

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
    glBindBuffer(targetGL, m_uGLhandle);

    // Try to use orphaning/renaming if the whole buffer is being reset
    const bool bResetWholeBuffer = (uOffsetElements == 0 && uNumElements == 0) ||
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
    ASSERT_M(isValid(), "Tried to lock an uninitialized buffer");
    ASSERT_M(!isLocked(), "Buffer is already locked");
    ASSERT_M(uOffsetElements < m_clParameters.uNumElements, "Invalid lock-range provided");

    const uint offsetBytes = uOffsetElements * m_clParameters.uElementSizeBytes;
    const uint rangeBytes = uNumElements * m_clParameters.uElementSizeBytes;
    const GLenum eLockOptionsGL = internal::mapLockOption(eLockOption);

    GLint origBoundBuffer;
    glGetIntegerv(m_clParameters.eBindingQueryType, &origBoundBuffer);
    glBindBuffer(m_clParameters.eInitialBufferTargetGL, m_uGLhandle);

    void* ptr = glMapBufferRange(m_clParameters.eInitialBufferTargetGL, 
         offsetBytes, rangeBytes, eLockOptionsGL);

    glBindBuffer(m_clParameters.eInitialBufferTargetGL, origBoundBuffer);

    ASSERT_M(ptr, "Buffer-lock failed");
    return ptr;
  }

//---------------------------------------------------------------------------//
  void GpuBufferGL4::unlock()
  {
    ASSERT_M(isLocked(), "Tried to unlock and unlocked buffer");

    GLint origBoundBuffer;
    glGetIntegerv(m_clParameters.eBindingQueryType, &origBoundBuffer);
    glBindBuffer(m_clParameters.eInitialBufferTargetGL, m_uGLhandle);

    glUnmapBuffer(m_clParameters.eInitialBufferTargetGL);

    glBindBuffer(m_clParameters.eInitialBufferTargetGL, origBoundBuffer);
  }
//---------------------------------------------------------------------------//
} } } } // end of namespace FANCY::Core::Rendering::GL4