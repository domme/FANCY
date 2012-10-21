#include "engineqglwidget.h"
#include <glm\glm.hpp>
#include <freeglut.h>
#include <Rendering/GLRenderer.h>
#include <Light/Light.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>
#include <Debug/PerformanceCheck.h>
#include <Debug/StatsManager.h>


#include "Engine.h"

#include "Scene/SceneManager.h"

static Engine*			pEngine;
static SceneManager*	pSceneManager;
static Camera*			pCamera;
static StatsManager*	pStatsGUI;
static GLRenderer*		pRenderer;

static int iScreenWidth						= 600;
static int iScreenHeight					= 800;
static const float PI						= 3.1415926535897932f;
static const glm::vec4 c_v4AmbientColor		( 0.1f, 0.1f, 0.1f, 1.0f );
static const glm::vec4 c_v4ClearColor		( 0.0f, 0.0f, 0.0f, 0.0f );
static const bool bUseFrameCap				= false;
static const unsigned int uFrameMScap		= 16;
static const int iTimerRedrawID				= 0;
static int iLastElapsedTime					= 0;
static int iMouseXLast						= 0;
static int iMouseYLast						= 0;
static float fCameraSpeed					= 100.0f;

EngineQGLwidget::EngineQGLwidget(QWidget *parent)
	: QGLWidget(parent),
	m_pTextEditFrameDurations( NULL )
{
	m_clRenderTime = QTime::currentTime();
	m_clRenderTime.restart();

	setFocusPolicy( Qt::FocusPolicy::StrongFocus );

	
}

EngineQGLwidget::~EngineQGLwidget()
{

}

void EngineQGLwidget::initializeGL()
{
	m_pTextEditFrameDurations = parentWidget()->findChild<QTextEdit*>( "durationsTextEdit" );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	pEngine = &Engine::GetInstance();
	pEngine->Init( iScreenHeight, iScreenWidth, c_v4AmbientColor, c_v4ClearColor, 1.0f );

	pSceneManager = new SceneManager();
	pEngine->SetScene( pSceneManager );

	pRenderer = pEngine->GetRenderer();

	pCamera = pEngine->GetCurrentCamera();

	PerformanceCheck::SetCallDelay( 100 );
	PerformanceCheck::SetEnabled( true );
	PerformanceCheck::SetOutputFlags( PerfStatsOutput::SCREEN );

	pStatsGUI = &StatsManager::GetInstance();

	setupEngineScene();

	connect( &m_clFPStimer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
	m_clFPStimer.start( 16 );
	m_bGLcontextReady = true;
}


void EngineQGLwidget::setupEngineScene()
{
	pSceneManager->LoadAssetIntoScene( "Models/Sibenik/sibenik_omme.dae" );

	//Dir light
	SceneNode* pLightNode = pSceneManager->getRootNode()->createChildSceneNode( "DirlightNode" );
	pLightNode->setRotation( -PI/4.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	DirectionalLight* pDirLight = pSceneManager->createDirectionalLight( "DirLight1", glm::vec3( 1.0f, 1.0f, 1.0f ), 0.5f );
	pLightNode->AttatchLight( pDirLight );

	//Pointlight 1
	SceneNode* pPointLightNode1 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode" );
	pPointLightNode1->setTranslation( glm::vec3( 0.4f, -13.0f, -5.7f ) );
	PointLight* pPointLight = pSceneManager->createPointLight( "Pointlight1", glm::vec3( 0.0f, 0.0f, 1.0f ), 2.5f, 1.0f, 5.0f );
	pPointLightNode1->AttatchLight( pPointLight );

	//Pointlight 2
	SceneNode* pPointLightNode2 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode2" );
	pPointLightNode2->setTranslation( glm::vec3( -17.25f, -13.7f, 6.6f ) );
	PointLight* pPointLight2 = pSceneManager->createPointLight( "Pointlight2", glm::vec3( 1.0f, 0.0f, 0.0f ), 2.0f, 0.5f, 5.0f );
	pPointLightNode2->AttatchLight( pPointLight2 );

	//Pointlight 3
	SceneNode* pPointLightNode3 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode3" );
	pPointLightNode3->setTranslation( glm::vec3( 8.7f, -10.0f, 6.7f ) );
	PointLight* pPointLight3 = pSceneManager->createPointLight( "Pointlight3", glm::vec3( 1.0f, 0.0f, 0.0f ), 1.5f, 3.0f, 7.0f );
	pPointLightNode3->AttatchLight( pPointLight3 );

	//Pointlight 4
	SceneNode* pPointLightNode4 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode4" );
	pPointLightNode4->setTranslation( glm::vec3( 17.7f, -8.3f, -0.05f ) );
	PointLight* pPointLight4 = pSceneManager->createPointLight( "Pointlight4", glm::vec3( 1.0f, 1.0f, 1.0f ), 1.2f, 3.0f, 7.0f );
	pPointLightNode4->AttatchLight( pPointLight4 );


	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_COLOR_GLOSS );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_NORMAL );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_DEPTH );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_SPECULAR );
	pEngine->AddDebugTexturePass( TextureSemantics::LIGHTING );
	pEngine->AddDebugTexturePass( TextureSemantics::LUMINANCE );
}


void EngineQGLwidget::resizeGL( int width, int height )
{
	iScreenWidth = width;
	iScreenHeight = height;
	glViewport( 0, 0, width, height );
	pEngine->SetResolution( width, height );
	pStatsGUI->SetViewportSize( glm::vec2( width, height ) );
	pStatsGUI->SetScreenPosition( glm::vec2( 5.0f, height - 10.0f ) );
}

void EngineQGLwidget::onPaintGL()
{
	repaint();
}

void EngineQGLwidget::drawStats()
{
	if( !PerformanceCheck::HasUpdates() )
		return;


	std::string szStatsString;

	const std::vector<String>& vStatsLines = pStatsGUI->GetStatsLines();

	for( int i = 0; i < vStatsLines.size(); ++i )
	{
		szStatsString += vStatsLines[ i ] + "\n";
	}

	m_pTextEditFrameDurations->setText( QString( szStatsString.c_str() ) );
	pStatsGUI->Clear();
}

void EngineQGLwidget::paintGL()
{
	int iCurrElapsedTime = glutGet( GLUT_ELAPSED_TIME );
	int iDeltaTime = iCurrElapsedTime - iLastElapsedTime;
	iLastElapsedTime = iCurrElapsedTime;

	PerformanceCheck::FrameStart();
	pStatsGUI->AddGuiLineValue( "FPS: ", pEngine->GetFPS() );

	pEngine->Update( iDeltaTime );
	pEngine->Render( iDeltaTime );

	drawStats();
}

void EngineQGLwidget::mouseMoveEvent( QMouseEvent* event )
{
	if( !m_bGLcontextReady )
		return;

	int dx = ( event->x() - m_iLastMouseX );
	int dy = ( event->y() - m_iLastMouseY );

	m_iLastMouseX = event->x();
	m_iLastMouseY = event->y();

	if( m_bMousePressed )
		pEngine->GetCurrentCamera()->RotateFromMouseMove( dx, dy );

	
}

void EngineQGLwidget::mousePressEvent( QMouseEvent* event )
{
	if( m_bMousePressed )
		return;

	m_iLastMouseX = event->x();
	m_iLastMouseY = event->y();
	m_bMousePressed = true;
	
}

void EngineQGLwidget::mouseReleaseEvent( QMouseEvent* event )
{
	if( !m_bMousePressed )
		return;

	m_bMousePressed = false;
}

void EngineQGLwidget::keyPressEvent( QKeyEvent* event )
{
	float fRealCameraSpeed = fCameraSpeed * pEngine->GetMovementMul();

	switch( event->key() )
	{
	case Qt::Key_W:
		{
			if( m_bMousePressed )
				pEngine->GetCurrentCamera()->MoveForward( fRealCameraSpeed );
		}
		break;

	case Qt::Key_S:
		{
			if( m_bMousePressed )
				pEngine->GetCurrentCamera()->MoveForward( -fRealCameraSpeed );

		}
		break;

	case Qt::Key_A:
		{
			if( m_bMousePressed )
				pEngine->GetCurrentCamera()->MoveSideways( -fRealCameraSpeed );

		}
		break;

	case Qt::Key_D:
		{
			if( m_bMousePressed )
				pEngine->GetCurrentCamera()->MoveSideways( fRealCameraSpeed );

		}
		break;
	}
	
}

void EngineQGLwidget::keyReleaseEvent( QKeyEvent* event )
{
	
}

void EngineQGLwidget::onToggleDebugDisplay( int iCheckState )
{
	pRenderer->SetDebugTexturesVisible( iCheckState == Qt::Checked );
}

void EngineQGLwidget::onToggleFXAA( int iCheckState )
{
	pRenderer->SetFXAAenabled( iCheckState == Qt::Checked );
}

void EngineQGLwidget::onToggleBloom( int iCheckState )
{
	pRenderer->SetUseBloom( iCheckState == Qt::Checked );
}

void EngineQGLwidget::onToggleHDR( int iCheckState )
{
	pRenderer->SetToneMappingEnabled( iCheckState == Qt::Checked );
}
