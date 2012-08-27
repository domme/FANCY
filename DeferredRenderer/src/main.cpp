#include <glew.h>
#include <glm\glm.hpp>
#include <freeglut.h>

#include "Engine.h"
#include "Scene/SceneManager.h"

static Engine*			pEngine;
static SceneManager*	pSceneManager;

static const int SCREEN_HEIGHT = 600;
static const int SCREEN_WIDTH = 800;
static const glm::vec4 c_v4AmbientColor( 0.2f, 0.2f, 0.2f, 1.0f );
static const glm::vec4 c_v4ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );


void initEngine()
{
	pEngine = &Engine::GetInstance();
	pEngine->Init( SCREEN_WIDTH, SCREEN_HEIGHT, c_v4AmbientColor, c_v4ClearColor, 1.0f );

	pSceneManager = new SceneManager();
	pEngine->SetScene( pSceneManager );
}

void render()
{
	glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	glutSwapBuffers();
}

void reshape( int width, int height )
{
	glViewport( 0, 0, width, height );

	
}


int main( int argc, char** argv )
{
	glewInit();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,600);
	glutCreateWindow("Deferred Rendering");
	glutDisplayFunc(render);
	glutReshapeFunc( reshape );

	initEngine();

	glutMainLoop();

	return 1;
}