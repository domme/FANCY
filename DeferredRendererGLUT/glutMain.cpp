/*#include <glew.h> 
#include <glm\glm.hpp>
#include <freeglut.h>

#include <Includes.h>

#include <Rendering/GLRenderer.h>
#include <Light/Light.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>
#include <Debug/PerformanceCheck.h>
#include <Debug/StatsManager.h>

#include <Engine.h>

#include <Scene/SceneManager.h>


static Engine*			pEngine;
static SceneManager*	pSceneManager;
static Camera*			pCamera;
static StatsManager*	pStatsGUI;
static GLRenderer*		pRenderer;
static SceneNode*		pDirlightNode;

static int iScreenWidth						= 600;
static int iScreenHeight					= 800;
static const float PI						= 3.1415926535897932f;
static const glm::vec4 c_v4AmbientColor		( 0.0f, 0.0f, 0.0f, 1.0f );
static const glm::vec4 c_v4ClearColor		( 0.0f, 0.0f, 0.0f, 0.0f );
static int iMouseXLast						= 0;
static int iMouseYLast						= 0;
static float fCameraSpeed					= 10.0f;
static int iLastElapsedTime					= 0;

static bool bMoveForward					= false;
static bool bMoveBackward					= false;
static bool bMoveLeft						= false;
static bool bMoveRight						= false;



void setupSibenikScene()
{
	pSceneManager->LoadAssetIntoScene( "Models/Sibenik/sibenik_omme.dae" );

	//Dir light
	pDirlightNode = pSceneManager->getRootNode()->createChildSceneNode( "DirlightNode" );
	pDirlightNode->setRotation( PI/4.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	DirectionalLight* pDirLight = pSceneManager->createDirectionalLight( "DirLight1", glm::vec3( 1.0f, 1.0f, 1.0f ), 0.5f );
//	pDirlightNode->AttatchLight( pDirLight );

	//Pointlight 1
	SceneNode* pPointLightNode1 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode" );
	pPointLightNode1->setTranslation( glm::vec3( 0.4f, -13.0f, -5.7f ) );
	PointLight* pPointLight = pSceneManager->createPointLight( "Pointlight1", glm::vec3( 0.0f, 0.0f, 1.0f ), 2.5f, 1.0f, 5.0f );
	pPointLightNode1->AttatchLight( pPointLight );

	/*
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
	
}

void setupEngineScene()
{
	//pSceneManager->LoadAssetIntoScene( "Models/Test_LightCamera.dae" );
	setupSibenikScene();

	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_COLOR_GLOSS );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_NORMAL );
	//pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_DEPTH );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_SPECULAR);
	pEngine->AddDebugTexturePass( TextureSemantics::LIGHTING );
	pEngine->AddDebugTexturePass( TextureSemantics::LUMINANCE );
}


void init()
{
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
	PerformanceCheck::SetOutputFlags( PerfStatsOutput::DEBUG_CONSOLE );

	pStatsGUI = &StatsManager::GetInstance();

	setupEngineScene();

}


void update()
{
	pDirlightNode->rotate( 0.005f, glm::vec3( 0.0f, 1.0f, 0.0f ) );
}

void onDisplay()
{
	PerformanceCheck::FrameStart();

	int iCurrElapsedTime = glutGet( GLUT_ELAPSED_TIME );
	int iDeltaTime = iCurrElapsedTime - iLastElapsedTime;
	iLastElapsedTime = iCurrElapsedTime;

	//processInputs();

	update();
	pEngine->Update( iDeltaTime );
	pEngine->Render( iDeltaTime );

	//drawStats();

	glutSwapBuffers();
}


void onResize( int width, int height )
{
	iScreenWidth = width;
	iScreenHeight = height;
	glViewport( 0, 0, width, height );
	pEngine->SetResolution( width, height );
	pStatsGUI->SetViewportSize( glm::vec2( width, height ) );
	pStatsGUI->SetScreenPosition( glm::vec2( 5.0f, height - 10.0f ) );
}

void onIdle()
{
	glutPostRedisplay();
}


int main( int argc, char* argv[] )
{
	glewInit();
	

	glutInitContextVersion( 3, 3 );
	glutInitContextFlags( GLUT_FORWARD_COMPATIBLE );
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInit( &argc, argv );

	glutCreateWindow( "DeferredRenderer GLUT Window" );
	glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition( 100,  100 );
	glutInitWindowSize( 800, 600 );
	
	glutDisplayFunc( onDisplay );
	glutReshapeFunc( onResize );
	glutIdleFunc( onIdle );

	init();

	glutMainLoop();

	return 1;
}

*/



#include <glew.h>
#include <glm\glm.hpp>
#include <freeglut.h>
#include <Includes.h>
#include <Rendering/GLRenderer.h>
#include <Light/Light.h>
#include <Light/DirectionalLight.h>
#include <Light/PointLight.h>
#include <Light/SpotLight.h>

#include "Engine.h"

#include "Scene/SceneManager.h"

struct SKeyState
{
	bool vbKeysDown[ 256 ];
};

static Engine*			pEngine;
static SceneManager*	pSceneManager;
static Camera*			pCamera;

static int iScreenWidth						= 600;
static int iScreenHeight					= 800;
static const float PI						= 3.1415926535897932f;
static const glm::vec4 c_v4AmbientColor		( 0.0f, 0.0f, 0.0f, 0.0f );
static const glm::vec4 c_v4ClearColor		( 0.0f, 0.0f, 0.0f, 0.0f );
static const bool bUseFrameCap				= false;
static const unsigned int uFrameMScap		= 16;
static const int iTimerRedrawID				= 0;
static int iLastElapsedTime					= 0;
static int iMouseXLast						= 0;
static int iMouseYLast						= 0;
static float fCameraSpeed					= 10.0f;
static SKeyState clKeyState;
static SceneNode*		pDirlightNode;


void initEngine()
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
	//pCamera->SetProjection( glm::perspectiveFov( glm::radians( 90.0f ), (float) iScreenWidth, (float) iScreenHeight, 1.0f, 200.0f ) );

	for( int i = 0; i < ARRAY_LENGTH( clKeyState.vbKeysDown ); ++i )
	{
		clKeyState.vbKeysDown[ i ] = false;
	}

	pEngine->GetRenderer()->SetDebugTexturesVisible( true );
}

void setupScene()
{
	pSceneManager->LoadAssetIntoScene( "Models/Sibenik/sibenik_omme.dae" );


	//Dir light
	pDirlightNode = pSceneManager->getRootNode()->createChildSceneNode( "DirlightNode" );
	pDirlightNode->setRotation( PI/4.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	DirectionalLight* pDirLight = pSceneManager->createDirectionalLight( "DirLight1", glm::vec3( 1.0f, 1.0f, 1.0f ), 0.5f );
//	pDirlightNode->AttatchLight( pDirLight );

	//Pointlight 1
	SceneNode* pPointLightNode1 = pSceneManager->getRootNode()->createChildSceneNode( "PointlightNode" );
	pPointLightNode1->setTranslation( glm::vec3( 0.4f, -13.0f, -5.7f ) );
	//pPointLightNode1->setTranslation( glm::vec3( 0.0f, 0.0f, 0.0f ) );

	PointLight* pPointLight = pSceneManager->createPointLight( "Pointlight1", glm::vec3( 0.0f, 0.0f, 1.0f ), 1.0f, 9.5f, 10.0f );
	pPointLightNode1->AttatchLight( pPointLight );

	/*
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
	*/

	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_COLOR_GLOSS );
	pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_NORMAL );
	//pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_DEPTH );
	//pEngine->AddDebugTexturePass( TextureSemantics::GBUFFER_SPECULAR );
	pEngine->AddDebugTexturePass( TextureSemantics::LIGHTING );
	//pEngine->AddDebugTexturePass( TextureSemantics::LUMINANCE );
}

void onMouseMoveActive( int x, int y )
{
	pCamera->RotateFromMouseMove( x - iMouseXLast, y - iMouseYLast );
	iMouseXLast = x;
	iMouseYLast = y;
}

void onMouseMovePassive( int x, int y )
{
	iMouseXLast = x;
	iMouseYLast = y;
}

void processInputs()
{
	float fRealCameraSpeed = fCameraSpeed * pEngine->GetMovementMul();

	if( clKeyState.vbKeysDown[ 'w' ] )
		pCamera->MoveForward( fRealCameraSpeed );

	if( clKeyState.vbKeysDown[ 's' ] )
		pCamera->MoveForward( -fRealCameraSpeed );

	if( clKeyState.vbKeysDown[ 'a' ] )
		pCamera->MoveSideways( -fRealCameraSpeed );

	if( clKeyState.vbKeysDown[ 'd' ] )
		pCamera->MoveSideways( fRealCameraSpeed );
}

void onKeyDown( unsigned char key, int iMouseX, int iMouseY )
{
	if( key < ARRAY_LENGTH( clKeyState.vbKeysDown ) )
		clKeyState.vbKeysDown[ key ] = true;
}

void onKeyReleased( unsigned char key, int iMouseX, int iMouseY )
{
	if( key < ARRAY_LENGTH( clKeyState.vbKeysDown ) )
		clKeyState.vbKeysDown[ key ] = false;

	if( key == 9 ) //TAB
		pEngine->GetRenderer()->SetDebugTexturesVisible( !pEngine->GetRenderer()->GetDebugTexturesVisible() );
}

void OnMouseClick(int button, int state, int x, int y)
{

}

void drawTextOverlays()
{
	glClear( GL_DEPTH_BUFFER_BIT );

	std::stringstream ss;
	ss << "FPS: " << pEngine->GetFPS();

	glUseProgram( 0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0, iScreenWidth, 0, iScreenHeight );
	
	glRasterPos2i( 5, iScreenHeight - 10 );
	glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
	
	glutBitmapString( GLUT_BITMAP_HELVETICA_12, (const unsigned char*) ss.str().c_str() );
	//glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, ss.str().c_str()[ 0 ] );
}

void render()
{
	int iCurrElapsedTime = glutGet( GLUT_ELAPSED_TIME );
	int iDeltaTime = iCurrElapsedTime - iLastElapsedTime;
	iLastElapsedTime = iCurrElapsedTime;
	
	processInputs();

	pEngine->Update( iDeltaTime );
	pEngine->Render( iDeltaTime );

	drawTextOverlays();

	glFlush();
	glutSwapBuffers();
}

void reshape( int width, int height )
{
	iScreenWidth = width;
	iScreenHeight = height;
	glViewport( 0, 0, width, height );
	pEngine->SetResolution( width, height );
}

void onIdle()
{
	glutPostRedisplay();
}

void onTimer( int iID )
{
	if( iID == iTimerRedrawID )
		glutPostRedisplay();
}


int main( int argc, char** argv )
{
	glewInit();

	glutInit(&argc, argv );
	glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(iScreenHeight, iScreenWidth );
	glutCreateWindow("Deferred Rendering");
	
	if( bUseFrameCap )
		glutTimerFunc( uFrameMScap, onTimer, iTimerRedrawID );
	else
		glutIdleFunc( onIdle );
	
	glutDisplayFunc(render);
	glutReshapeFunc( reshape );
	glutMotionFunc( onMouseMoveActive );
	glutPassiveMotionFunc( onMouseMovePassive );
	glutKeyboardUpFunc( onKeyReleased );
	glutKeyboardFunc( onKeyDown );

	initEngine();
	setupScene();

	glutMainLoop();

	return 1;
}


