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

	for( int i = 0; i < ARRAY_LENGTH( clKeyState.vbKeysDown ); ++i )
	{
		clKeyState.vbKeysDown[ i ] = false;
	}
}

void setupScene()
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