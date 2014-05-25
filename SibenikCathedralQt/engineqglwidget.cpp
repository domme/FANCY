#include "engineqglwidget.h"
#include <glm\glm.hpp>
#include <Rendering/GLRenderer.h>
#include <Light/Light.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>
#include <Debug/PerformanceCheck.h>
#include <Debug/StatsManager.h>

#include "Engine.h"

#include "Scene/SceneManager.h"

static Engine*			pEngine = NULL;
static SceneManager*	pSceneManager = NULL;
static StatsManager*	pStatsGUI = NULL;
static GLRenderer*		pRenderer = NULL;
static SceneNode*		pDirlightNode = NULL;
static SceneNode* pPointLightNode1 = NULL;
PointLight* pPointLight = NULL;

static int iScreenWidth						= 600;
static int iScreenHeight					= 800;
static const float PI						= 3.1415926535897932f;
static const glm::vec4 c_v4AmbientColor		( 0.4f, 0.4f, 0.4f, 1.0f );
static const glm::vec4 c_v4ClearColor		( 0.0f, 0.0f, 0.0f, 0.0f );
static const bool bUseFrameCap				= false;
static const unsigned int uFrameMScap		= 16;
static const int iTimerRedrawID				= 0;
static int iLastElapsedTime					= 0;
static int iMouseXLast						= 0;
static int iMouseYLast						= 0;
static float fCameraSpeed					= 10.0f;

static bool bMoveForward					= false;
static bool bMoveBackward					= false;
static bool bMoveLeft						= false;
static bool bMoveRight						= false;

EngineQGLwidget::EngineQGLwidget(QWidget *parent)
	: QGLWidget(parent),
	m_pTableFrameDurations( NULL ),
	m_pStatsItemModel( NULL )
{
	m_clRenderTime = QTime::currentTime();
	m_clRenderTime.restart();

	setFocusPolicy( Qt::FocusPolicy::StrongFocus );

	
}

EngineQGLwidget::~EngineQGLwidget()
{
	SAFE_DELETE( m_pStatsItemModel );
}

void EngineQGLwidget::initializeGL()
{
	glewInit();

	m_pTableFrameDurations = parentWidget()->findChild<QTableView*>( "statsTableView" );
	m_pStatsItemModel = new QStandardItemModel( 0, 2, this );
	m_pStatsItemModel->setHorizontalHeaderItem( 0, new QStandardItem( QString( "Process" ) ) );
	m_pStatsItemModel->setHorizontalHeaderItem( 1, new QStandardItem( QString( "Duration (ms)" ) ) );
	m_pTableFrameDurations->setModel( m_pStatsItemModel );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	pEngine = &Engine::GetInstance();
	pEngine->Init( iScreenHeight, iScreenWidth, c_v4AmbientColor, c_v4ClearColor, 1.0f );

	pSceneManager = new SceneManager();
	pEngine->SetScene( pSceneManager );

	pRenderer = pEngine->GetRenderer();


	PerformanceCheck::SetCallDelay( 100 );
	PerformanceCheck::SetEnabled( true );
	PerformanceCheck::SetOutputFlags( PerfStatsOutput::SCREEN );

	pStatsGUI = &StatsManager::GetInstance();

	//setupEngineScene();
	setupTestScene();

	connect( &m_clFPStimer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
	m_clFPStimer.start( 16 );
	m_bGLcontextReady = true;
}

void EngineQGLwidget::setupTestScene()
{
	pSceneManager->LoadAssetIntoScene( "Models/Test_LightCamera.dae" );
	//pSceneManager->LoadAssetIntoScene( "Models/BlenderTestExport.dae" );
	
	/*pDirlightNode = pSceneManager->getRootNode()->createChildSceneNode( "DirlightNode" );
	pDirlightNode->setRotation( PI/4.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	DirectionalLight* pDirLight = pSceneManager->createDirectionalLight( "DirLight1", glm::vec3( 1.0f, 1.0f, 1.0f ), 0.2f );
	pDirlightNode->AttatchLight( pDirLight ); */
}

void EngineQGLwidget::setupSibenikScene()
{
	pSceneManager->LoadAssetIntoScene( "Models/Sibenik/sibenik_omme.dae" );
	
	//Dir light
	pDirlightNode = pSceneManager->getRootNode()->createChildSceneNode( "DirlightNode" );
	pDirlightNode->setRotation( PI/4.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	DirectionalLight* pDirLight = pSceneManager->createDirectionalLight( "DirLight1", glm::vec3( 1.0f, 1.0f, 1.0f ), 0.2f );
	//pDirlightNode->AttatchLight( pDirLight );

	//Pointlight 1
	pPointLightNode1 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode" );
	pPointLightNode1->setTranslation( glm::vec3( 0.4f, -12.0f, -5.7f ) );
	pPointLight = pSceneManager->createPointLight( "Pointlight1", glm::vec3( 0.0f, 0.0f, 1.0f ), 2.0f, 9.0f, 10.0f );
	pPointLightNode1->AttatchLight( pPointLight );

	///*
	//Pointlight 2
	SceneNode* pPointLightNode2 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode2" );
	pPointLightNode2->setTranslation( glm::vec3( -17.25f, -13.7f, 6.6f ) );
	PointLight* pPointLight2 = pSceneManager->createPointLight( "Pointlight2", glm::vec3( 1.0f, 0.0f, 0.0f ), 2.0f, 8.0f, 10.0f );
	pPointLightNode2->AttatchLight( pPointLight2 );

	//Pointlight 3
	SceneNode* pPointLightNode3 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode3" );
	pPointLightNode3->setTranslation( glm::vec3( 8.7f, -10.0f, 6.7f ) );
	PointLight* pPointLight3 = pSceneManager->createPointLight( "Pointlight3", glm::vec3( 1.0f, 0.0f, 0.0f ), 1.5f, 6.0f, 7.0f );
	pPointLightNode3->AttatchLight( pPointLight3 );

	//Pointlight 4
	SceneNode* pPointLightNode4 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode4" );
	pPointLightNode4->setTranslation( glm::vec3( 10.7f, -8.3f, -0.05f ) );
	PointLight* pPointLight4 = pSceneManager->createPointLight( "Pointlight4", glm::vec3( 1.0f, 1.0f, 1.0f ), 1.2f, 8.5f, 9.0f );
	pPointLightNode4->AttatchLight( pPointLight4 );
	//*/
	
}


void EngineQGLwidget::setupEngineScene()
{
	//pSceneManager->LoadAssetIntoScene( "Models/Test_LightCamera.dae" );
	setupSibenikScene();
	//setupTestScene();

	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_COLOR_GLOSS );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_NORMAL );
	//pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_DEPTH );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_SPECULAR);
	pEngine->AddDebugTexturePass( TextureSemantics::LIGHTING );
	pEngine->AddDebugTexturePass( TextureSemantics::LUMINANCE );
}


void EngineQGLwidget::resizeGL( int width, int height )
{
	iScreenWidth = width;
	iScreenHeight = height;
	pEngine->SetResolution( width, height );
	pStatsGUI->SetViewportSize( glm::vec2( width, height ) );
	pStatsGUI->SetScreenPosition( glm::vec2( 5.0f, height - 10.0f ) );
}

void EngineQGLwidget::onPaintGL()
{
	repaint();
	swapBuffers();
}

void EngineQGLwidget::drawStats()
{
	if( !PerformanceCheck::HasUpdates() )
		return;

	pStatsGUI->AddGuiLineValue( "FPS: ", pEngine->GetFPS() );
	const std::vector<SStatsEntry>& vStats = pStatsGUI->GetStats();

	for( int i = 0; i < vStats.size(); ++i )
	{
		std::stringstream ss;
		ss.precision( 5 );
		ss << vStats[ i ].m_f64Value;

		m_pStatsItemModel->setItem( i, 0, new QStandardItem( QString( vStats[ i ].m_szMessage.c_str() ) ) );
		m_pStatsItemModel->setItem( i, 1, new QStandardItem( QString( ss.str().c_str() ) ) );
	}
	
	//m_pTextEditFrameDurations->setText( QString( szStatsString.c_str() ) );
	pStatsGUI->Clear();
}

void EngineQGLwidget::processInputs()
{
	float fRealCameraSpeed = fCameraSpeed * pEngine->GetMovementMul();
	Camera* pCamera = pSceneManager->GetCamera();

	if( bMoveForward )
		pCamera->MoveForward( fRealCameraSpeed );

	if( bMoveBackward )
		pCamera->MoveForward( -fRealCameraSpeed );

	if( bMoveLeft )
		pCamera->MoveSideways( -fRealCameraSpeed );

	if( bMoveRight )
		pCamera->MoveSideways( fRealCameraSpeed );
}

void EngineQGLwidget::update()
{
	if( pDirlightNode )
		pDirlightNode->rotate( 0.005f, glm::vec3( 0.0f, 1.0f, 0.0f ) );


	static float x = -0.02f;
	static float fXPos = 0.4f;

	if( fXPos < -17.0f )
		x = 0.02f;

	if( fXPos > 0.4f )
		x = -0.02f;

	fXPos += x;
	
	if( pPointLight && pPointLightNode1 )
	{
		pPointLightNode1->translate( glm::vec3( x, 0.0f, 0.0f ) );
		pPointLight->SetDirty( true );
	}
	
	
}

void EngineQGLwidget::paintGL()
{
	PerformanceCheck::FrameStart();

	int iCurrElapsedTime = 17; //glutGet( GLUT_ELAPSED_TIME );
	int iDeltaTime = iCurrElapsedTime - iLastElapsedTime;
	iLastElapsedTime = iCurrElapsedTime;

	processInputs();

	update();
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
		pSceneManager->GetCamera()->RotateFromMouseMove( dx, dy );
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
	
	
	switch( event->key() )
	{
	case Qt::Key_W:
		{
			if( m_bMousePressed )
				bMoveForward = true;
		}
		break;

	case Qt::Key_S:
		{
			if( m_bMousePressed )
				bMoveBackward = true;

		}
		break;

	case Qt::Key_A:
		{
			if( m_bMousePressed )
				bMoveLeft = true;

		}
		break;

	case Qt::Key_D:
		{
			if( m_bMousePressed )
				bMoveRight = true;

		}
		break;
	}
	
}

void EngineQGLwidget::keyReleaseEvent( QKeyEvent* event )
{
	switch( event->key() )
	{
	case Qt::Key_W:
		{
			bMoveForward = false;		
		}
		break;

	case Qt::Key_S:
		{
			bMoveBackward = false;

		}
		break;

	case Qt::Key_A:
		{
			bMoveLeft = false;
		}
		break;

	case Qt::Key_D:
		{
			bMoveRight = false;
		}
		break;
	}
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
