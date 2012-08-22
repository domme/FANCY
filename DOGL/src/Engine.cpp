#include "includes.h"

#include "Engine.h"
#include "Scene/Camera.h"
#include "IO/PathService.h"

#include "Rendering/GLRenderer.h"

Engine::Engine() :
m_fCurrentFPS( 0.0f ), 
m_uCurrentElapsedTicksMS( 0 ),
m_uCurrentFrameCount( 0 ),
m_pRenderCamera( NULL ),
m_pRenderer( NULL ),
m_uNumMeshes( 1 ),
m_bInitialized( false ),
m_bShowDebugTextures( false ),
m_bUseFXAA( true ),
m_bUseToneMapping( true ),
m_bUseBloom( true ), 
m_fHDRexposure( 0.5f ),
m_fHDRlightAdaption( 0.04f )
{

}

Engine::~Engine()
{
	SAFE_DELETE( m_pRenderCamera ); 
}

Engine& Engine::GetInstance()
{
	static Engine instance;
	return instance;
}


void Engine::Update( const uint elapsedTicksMS )
{
	UpdateFPS( elapsedTicksMS );
	
	//m_pCameraController->Update();
	m_pRenderCamera->Update( elapsedTicksMS );
}


void Engine::Render( const uint elapsedTicksMS )
{
	m_uCurrentFrameCount++; //increment framecount for fps calculations
}


void Engine::Init( uint uScreenWidth, uint uScreenHeight, const glm::vec4& v4AmbientColor, const glm::vec4& v4ClearColor, float fClearDepth )
{
	if( m_bInitialized )
	{
		//don't let the engine be initialized more than once - would really lead to confusing behavior ;-)
		return;
	}

	glewInit();

	m_uScreenWidth = uScreenWidth;
	m_uScreenHeight = uScreenHeight;

	m_v4AmbientColor = v4AmbientColor;
	m_v4ClearColor = v4ClearColor;
	m_fClearDepth = fClearDepth;

	glClearColor( v4ClearColor.r, v4ClearColor.g, v4ClearColor.b, v4ClearColor.a );
	glClearDepth( fClearDepth );

	PathService::SetResourceLocation( "..\\..\\..\\Resources\\" );

	//Initialize a default camera for the scene. The camera can be overwritten from anywhere in the engine though
	m_pRenderCamera = new Camera();
	m_pRenderCamera->InitPerspectiveProjection( 60.0f, ( (float) uScreenWidth / (float) uScreenHeight ) , 1.0f, 200.0f );

    glm::vec3 v3Eye( 0.0f, 0.0f, 0.0f );
	glm::vec3 v3At( 0.0f, 0.0f, -1.0f );
	glm::vec3 v3Up( 0.0f, 1.0f, 0.0f );

	m_pRenderCamera->InitView( v3Eye, v3At, v3Up );

	//m_pCameraController = &CameraController::getInstance();
	//m_pCameraController->Init( m_pRenderCamera );

	m_pRenderer = &GLRenderer::GetInstance();
	m_pRenderer->init( uScreenWidth, uScreenHeight );
    
	m_bInitialized = true;
}

void Engine::SetResolution( uint uWidth, uint uHeight )
{
	if( !m_bInitialized )
		return;

	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;

	m_pRenderer->SetResolution( uWidth, uHeight );
	m_pRenderCamera->InitPerspectiveProjection( 60.0f, ( (float) uWidth / (float) uHeight ) , 1.0f, 2000.0f );
}

glm::mat4* Engine::GetWorldMat()
{
	return &m_gMatWorld;
}

Camera* Engine::GetCurrentCamera()
{
	return m_pRenderCamera;
}


void Engine::UpdateFPS( const uint elapsedTicksMS )
{
	m_uCurrentElapsedTicksMS += elapsedTicksMS;
	m_uAbsElapsedTicksMS += elapsedTicksMS;
	m_uDeltaTicksMS = elapsedTicksMS;
	 
	if(  m_uCurrentElapsedTicksMS > 1000 ) //1 sec is over
	{
		m_fCurrentFPS = m_uCurrentFrameCount;
		m_uCurrentFrameCount = 0;
		m_uCurrentElapsedTicksMS = 0;
	}
}

float Engine::GetFPS()
{
	return m_fCurrentFPS;
}
