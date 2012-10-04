#include "engineqglwidget.h"
#include <glm\glm.hpp>
#include <freeglut.h>
#include <Rendering/GLRenderer.h>
#include <Light/Light.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>
#include <Debug/PerformanceCheck.h>
#include <Debug/StatsGui.h>

#include "Engine.h"

#include "Scene/SceneManager.h"

struct SKeyState
{
	bool vbKeysDown[ 256 ];
};

static Engine*			pEngine;
static SceneManager*	pSceneManager;
static Camera*			pCamera;
static StatsGui*		pStatsGUI;

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
static float fCameraSpeed					= 10.0f;
static SKeyState clKeyState;

EngineQGLwidget::EngineQGLwidget(QWidget *parent)
	: QGLWidget(parent)
{

}

EngineQGLwidget::~EngineQGLwidget()
{

}

void EngineQGLwidget::initializeGL()
{
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	pEngine = &Engine::GetInstance();
	pEngine->Init( iScreenHeight, iScreenWidth, c_v4AmbientColor, c_v4ClearColor, 1.0f );

	pSceneManager = new SceneManager();
	pEngine->SetScene( pSceneManager );

	pCamera = pEngine->GetCurrentCamera();

	for( int i = 0; i < ARRAY_LENGTH( clKeyState.vbKeysDown ); ++i )
	{
		clKeyState.vbKeysDown[ i ] = false;
	}

	//PerformanceCheck::SetCallDelay( 100 );
	PerformanceCheck::SetEnabled( true );
	PerformanceCheck::SetOutputFlags( PerfStatsOutput::SCREEN );

	pStatsGUI = &StatsGui::GetInstance();

	setupEngineScene();
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

void EngineQGLwidget::paintGL()
{
	int iCurrElapsedTime = glutGet( GLUT_ELAPSED_TIME );
	int iDeltaTime = iCurrElapsedTime - iLastElapsedTime;
	iLastElapsedTime = iCurrElapsedTime;

	//pStatsGUI->Clear();
	//pStatsGUI->AddGuiLineValue( "FPS: ", pEngine->GetFPS() );

	//processInputs();

	pEngine->Update( iDeltaTime );
	pEngine->Render( iDeltaTime );

	//drawTextOverlays();

	//glFlush();
	//glutSwapBuffers();
}
