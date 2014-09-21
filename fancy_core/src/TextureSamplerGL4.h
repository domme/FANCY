#ifndef INCLUDE_TEXTURESAMPLERGL4_H
#define INCLUDE_TEXTURESAMPLERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "ObjectName.h"

namespace Fancy { namespace Core { namespace Rendering { namespace GL4 {
  //---------------------------------------------------------------------------//
  class TextureSamplerGL4
  {
  //---------------------------------------------------------------------------//
  public:
    TextureSamplerGL4();
    ~TextureSamplerGL4();
  //---------------------------------------------------------------------------//
    const ObjectName& getName() const {return m_Name;}
    GLuint getProgramHandle() const {return m_uHandleGL;}

    const TextureSamplerProperties& getProperties() const {return m_properties;}
    void create(const ObjectName& rName, const TextureSamplerProperties& rProperties);
  //---------------------------------------------------------------------------//
  protected:
    void destroy();

    /// Name of the shader as loaded from disk
    ObjectName m_Name;
    /// GL-internal handle to the program object
    GLuint m_uHandleGL;
    /// ShaderStage this program defines
    ShaderStage m_eShaderStage;
    /// Internal properties of the sampler defined at creation time
    TextureSamplerProperties m_properties;
  };
  //---------------------------------------------------------------------------//
} } } }  // end of namespace Fancy::Core::Rendering::GL4


#endif  // INCLUDE_GPUPROGRAMGL4_H