#ifndef INCLUDE_TEXTURESAMPLERGL4_H
#define INCLUDE_TEXTURESAMPLERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "OpenGLprerequisites.h"
#include "AdapterGL4.h"
#include "ObjectName.h"

namespace Fancy { namespace Rendering { namespace GL4 {
  //---------------------------------------------------------------------------//
  class TextureSamplerGL4
  {
  //---------------------------------------------------------------------------//
  public:
    TextureSamplerGL4();
    ~TextureSamplerGL4();
  //---------------------------------------------------------------------------//
    const ObjectName& getName() const {return m_Name;}
    GLuint getGLhandle() const {return m_uHandleGL;}

    const TextureSamplerProperties& getProperties() const {return m_properties;}
    void create(const ObjectName& rName, const TextureSamplerProperties& rProperties);
  //---------------------------------------------------------------------------//
  protected:
    void destroy();

    /// Name given to the sampler object by the api
    ObjectName m_Name;
    /// GL-internal handle to the OpenGL sampler object
    GLuint m_uHandleGL;
    /// Internal properties of the sampler defined at creation time
    TextureSamplerProperties m_properties;
  };
  //---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4

#endif

#endif  // INCLUDE_GPUPROGRAMGL4_H