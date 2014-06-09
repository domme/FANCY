#include <FancyCorePrerequisites.h>
#include <Camera.h>
#include <PathService.h>
#include <Renderer.h>
#include <PerformanceCheck.h>

#include "DeferredRenderer.h"
#include "Engine.h"

Engine::Engine() :
m_pDeferredRenderer( NULL ), 
m_fCurrentFPS( 0.0f ), 
m_uCurrentElapsedTicksMS( 0 ),
m_uCurrentFrameCount( 0 ),
m_pRenderer( NULL ),
m_pScene( NULL ),
m_uNumMeshes( 1 ),
m_bInitialized( false ),
m_eVolumeMode( Engine::VOLUMES_SHOW_BOTH ) 
{

}

Engine::~Engine()
{
  SAFE_DELETE( m_pScene );
}

Engine& Engine::GetInstance()
{
  static Engine instance;
  return instance;
}


void Engine::Update( const uint elapsedTicksMS )
{
  UpdateFPS( elapsedTicksMS );
}


void Engine::Render( const uint elapsedTicksMS )
{
  m_uCurrentFrameCount++; //increment framecount for fps calculations
  
  if( !m_pScene )
    return;

  CHECK_PERFORMANCE( m_pDeferredRenderer->RenderScene( m_pScene ), "RenderScene()" );
}


void Engine::Init( uint uScreenWidth, uint uScreenHeight, const glm::vec4& v4AmbientColor, const glm::vec4& v4ClearColor, float fClearDepth )
{
  if( m_bInitialized )
  {
    //don't let the engine be initialized more than once - would really lead to confusing behavior ;-)
    return;
  }

#ifdef __WINDOWS
  glewInit();
#endif

  int glMajor, glMinor;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
  glGetIntegerv(GL_MINOR_VERSION, &glMinor);

  LOG<std::string>((const char*) glGetString(GL_VENDOR));
  LOG<std::string>((const char*) glGetString(GL_RENDERER));
  LOG<std::string>((const char*) glGetString(GL_VERSION));
  LOG<std::string>((const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

  m_uScreenWidth = uScreenWidth;
  m_uScreenHeight = uScreenHeight;

  m_v4AmbientColor = v4AmbientColor;
  m_v4ClearColor = v4ClearColor;
  m_fClearDepth = fClearDepth;

  glClearColor( v4ClearColor.r, v4ClearColor.g, v4ClearColor.b, v4ClearColor.a );
  glClearDepth( fClearDepth );


  setRenderMode( RENDER_DEFERRED );
  
  //m_pCameraController = &CameraController::getInstance();
  //m_pCameraController->Init( m_pRenderCamera );

  m_pRenderer = &Renderer::GetInstance();
  m_pRenderer->init( uScreenWidth, uScreenHeight );
  //AddResolutionListener<GLRenderer>( m_pRenderer, &GLRenderer::OnResolutionChanged );

  m_pDeferredRenderer = &DeferredRenderAlgorithm::GetInstance();
  m_pDeferredRenderer->Init( m_uScreenWidth, m_uScreenHeight, m_pRenderer );
  //AddResolutionListener<GLDeferredRenderer>( m_pDeferredRenderer, (*m_pDeferredRenderer).OnResolutionChanged ); 
    
  m_bInitialized = true;
}

void Engine::SetResolution( uint uWidth, uint uHeight )
{
  if( !m_bInitialized )
    return;

  m_uScreenWidth = uWidth;
  m_uScreenHeight = uHeight;

  //m_delegateResolutionChanged.raiseEvent( glm::ivec2( uWidth, uHeight ) );

  //TODO: add them to the resolution-event instead!
  glm::ivec2 newRes ( uWidth, uHeight ); 
  m_pRenderer->OnResolutionChanged( newRes );
  m_pDeferredRenderer->OnResolutionChanged( newRes );
  m_pScene->OnResolutionChanged( newRes );
}

glm::mat4* Engine::GetWorldMat()
{
  return &m_gMatWorld;
}



void Engine::UpdateFPS( const uint elapsedTicksMS )
{
  m_uCurrentElapsedTicksMS += elapsedTicksMS;
  m_uAbsElapsedTicksMS += elapsedTicksMS;
  m_uDeltaTicksMS = elapsedTicksMS;
   
  if(  m_uCurrentElapsedTicksMS > 1000 ) //1 sec is over
  {
    m_fCurrentFPS = static_cast<float>(m_uCurrentFrameCount);
    m_uCurrentFrameCount = 0;
    m_uCurrentElapsedTicksMS = 0;
  }
}

float Engine::GetFPS()
{
  return m_fCurrentFPS;
}

//template<typename ObjectT>
//int Engine::AddResolutionListener( ObjectT* pListener, void (ObjectT::*callbackFunc) (glm::vec2) )
//{
//	m_delegateResolutionChanged.registerListener( pListener, callbackFunc );
//}
