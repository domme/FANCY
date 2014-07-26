#ifndef INCLUDE_GLUTIL_H
#define INCLUDE_GLUTIL_H

#include "RendererPrerequisites.h"

class GLutil {
  public:
    //-----------------------------------------------------------------------//
    static void setGlCap(const GLenum eCap, const bool bEnabled)
    {
      m_glEnableDisableFunc[static_cast<uint8>(bEnabled)](eCap);
    }
    //-----------------------------------------------------------------------//
    static void setGlCapI(const GLenum eCap, const GLuint index, bool bEnabled)
    {
      m_glEnableDisableIndexedFunc[static_cast<uint8>(bEnabled)](eCap, index);
    }
    //-----------------------------------------------------------------------//
  private:
    typedef void ( __stdcall *GlEnumFunc) (GLenum);
    typedef void ( __stdcall *GLEnumIndexFunc) (GLenum, GLuint);
    
    static GlEnumFunc m_glEnableDisableFunc[2];
    static GLEnumIndexFunc m_glEnableDisableIndexedFunc[2];
};


#endif  // INCLUDE_GLUTIL_H