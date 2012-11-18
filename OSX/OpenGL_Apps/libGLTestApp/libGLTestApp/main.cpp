//
//  main.cpp
//  libGL_GLUT
//
//  Created by Dominik on 28.09.12.
//  Copyright (c) 2012 Dominik. All rights reserved.
//

#include <iostream>
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>

#include "Rendering/GLRenderer.h"

GLRenderer* pRenderer;

void onDisplay()
{
    glClear( GL_COLOR_BUFFER_BIT );
    

    
    glFlush();
    glutSwapBuffers();
}

void onResize( int iWidth, int iHeight )
{
    glViewport( 0, 0, iWidth, iHeight );
}

void init()
{
    glClearColor( 1.0f, 0.0f, 0.0f, 0.0f );
    
    pRenderer = &GLRenderer::GetInstance();

    
}


int main(int argc, const char * argv[])
{
    glutInit( &argc, const_cast<char**>( argv ) );
    glutCreateWindow( "libGL_GLUT Window" );
    glutInitDisplayMode( GLUT_DOUBLE );
    glutInitWindowPosition( 100,  100 );
    glutInitWindowSize( 800, 600 );
    glutDisplayFunc( onDisplay );
    glutReshapeFunc( onResize );
    
    
    init();
    
    glutMainLoop();
    
    return 1;
    
}

