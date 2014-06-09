#include "StatsManager.h"


StatsManager::StatsManager(void) :
m_v2ScreenPosition( 0.0f, 0.0f ),
m_v2ViewportSize( 0.0f, 0.0f )
{
}


StatsManager::~StatsManager(void)
{
}

StatsManager& StatsManager::GetInstance()
{
	static StatsManager clInstance; 
	return clInstance;
}

void StatsManager::AddGuiLineValue( const String& szMessage, float fValue )
{
	SStatsEntry entry;
	entry.m_szMessage = szMessage;
	entry.m_f64Value = fValue;

	m_vStats.push_back( entry );
}

void StatsManager::AddGuiLineValue( const String& szMessage, double fValue )
{
	SStatsEntry entry;
	entry.m_szMessage = szMessage;
	entry.m_f64Value = fValue;

	m_vStats.push_back( entry );
}

void StatsManager::RenderGuiLinesGLUT()
{
  // TODO: Re-implement
//
//	glClear( GL_DEPTH_BUFFER_BIT );
//
//	glUseProgram( 0 );
//
//	glMatrixMode( GL_MODELVIEW );
//	glLoadIdentity();
//
//	glMatrixMode( GL_PROJECTION );
//	glLoadIdentity();
//	gluOrtho2D( 0, m_v2ViewportSize.x, 0, m_v2ViewportSize.y );
//	glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
//
//	for( int i = 0; i < m_vStats.size(); ++i )
//	{
//		std::stringstream ss;
//		ss << m_vStats[ i ].m_szMessage << " " << m_vStats[ i ].m_f64Value;
//
//		glRasterPos2i( static_cast<int32>(m_v2ScreenPosition.x), static_cast<int32>(m_v2ScreenPosition.y - 15 * i) );
//#ifdef __WINDOWS
//       // glutBitmapString( GLUT_BITMAP_HELVETICA_12, (const unsigned char*) ss.str().c_str() );
//#endif
//	}
}


