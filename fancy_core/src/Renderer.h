#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERER

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class Renderer : public PLATFORM_DEPENDENT_NAME(Renderer)
  {
    public:
      Renderer(void* aNativeWindowHandle) : PLATFORM_DEPENDENT_NAME(Renderer)(aNativeWindowHandle) {}
      virtual ~Renderer() {}

    protected:
      
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class Texture;
//---------------------------------------------------------------------------//
  class RenderCore : public PLATFORM_DEPENDENT_NAME(RenderCore)
  {
  public:
    /// Init platform-independent stuff
    static void Init();
    /// Called after the rendering system has been fully initialzed (Rendering-resources can be created here)
    static void PostInit();

    /// Shutdown platform-independent stuff
    static void Shutdown();

    static const Texture* GetDefaultDiffuseTexture() { return ourDefaultDiffuseTexture.get(); }
    static const Texture* GetDefaultNormalTexture() { return ourDefaultNormalTexture.get(); }
    static const Texture* GetDefaultSpecularTexture() { return ourDefaultSpecularTexture.get(); }

  private:
    RenderCore() {}

    static std::shared_ptr<Texture> ourDefaultDiffuseTexture;
    static std::shared_ptr<Texture> ourDefaultNormalTexture;
    static std::shared_ptr<Texture> ourDefaultSpecularTexture;
  };
//---------------------------------------------------------------------------//
} // end of namespace Rendering
} // end of namespace Fancy

#endif  // INCLUDE_RENDERER_H