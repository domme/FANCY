#include "StatsGui.h"


StatsGui::StatsGui(void) :
m_v2ScreenPosition( 0.0f, 0.0f ),
m_v2ViewportSize( 0.0f, 0.0f )
{
}


StatsGui::~StatsGui(void)
{
}

StatsGui& StatsGui::GetInstance()
{
	static StatsGui clInstance; 
	return clInstance;
}

void StatsGui::AddGuiLineValue( const String& szMessage, float fValue )
{
	std::stringstream ss;
	ss << szMessage << fValue;
	m_vGuiLines.push_back( ss.str() );
}

void StatsGui::AddGuiLineValue( const String& szMessage, double fValue )
{
	std::stringstream ss;
	ss << szMessage << fValue;
	m_vGuiLines.push_back( ss.str() );
}

void StatsGui::RenderGuiLinesGLUT()
{
	glClear( GL_DEPTH_BUFFER_BIT );

	glUseProgram( 0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0, m_v2ViewportSize.x, 0, m_v2ViewportSize.y );
	glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );

	for( int i = 0; i < m_vGuiLines.size(); ++i )
	{
		glRasterPos2i( m_v2ScreenPosition.x, m_v2ScreenPosition.y - 15 * i );
		glutBitmapString( GLUT_BITMAP_HELVETICA_12, (const unsigned char*) m_vGuiLines[ i ].c_str() );
	}
}


