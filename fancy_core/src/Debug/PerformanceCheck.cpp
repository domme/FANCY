#include "PerformanceCheck.h"

#include "StatsManager.h"

uint32 PerformanceCheck::m_u32CallDelay = 0;
uint32 PerformanceCheck::m_u32CallCount = 0;
bool	PerformanceCheck::m_bEnable = false;
uint32 PerformanceCheck::m_u32OutputFlags = PerfStatsOutput::DEBUG_CONSOLE;

PerformanceCheck::PerformanceCheck() :
m_u64StartTime( 0 ),
m_szMessage( "" )
{
	
}

void PerformanceCheck::FrameStart()
{
	if( m_u32CallCount > m_u32CallDelay )
		m_u32CallCount = 0;
	
	++m_u32CallCount;
}

void PerformanceCheck::StartPerformanceCheck( const String& szMessage )
{
#ifdef __WINDOWS
    if( !m_bEnable )
		return;

	if( m_u32CallDelay > 0 && m_u32CallCount < m_u32CallDelay )
		return;

	LARGE_INTEGER iTicks;

	QueryPerformanceCounter( &iTicks );
	uint64 u64Ticks =iTicks.QuadPart;

	m_u64StartTime = u64Ticks;
	m_szMessage = szMessage;
#endif
}


void PerformanceCheck::FinishAndLogPerformanceCheck()
{
#ifdef __WINDOWS
    if( !m_bEnable )
		return;

	if( m_u32CallDelay > 0 && m_u32CallCount < m_u32CallDelay )
		return;

	LARGE_INTEGER iFreq;
	
	QueryPerformanceFrequency( &iFreq );
	
	LARGE_INTEGER iTicks;
	
	QueryPerformanceCounter( &iTicks );
	
	uint64 u64End = iTicks.QuadPart;
	uint64 u64Freq = iFreq.QuadPart;
	
	
	uint64 u64duration = u64End - m_u64StartTime;
	
	double f64Delay = (double) u64duration / ( (double) u64Freq / 1000.0 );
		
	//char cbuf[1024];
	
	//sprintf( cbuf, "%s %.20f \n", pMsg, f64Delay );
	//fprintf(stdout, "%s", cbuf);

	if( m_u32OutputFlags & PerfStatsOutput::DEBUG_CONSOLE )
		logPerformanceCheck_CONSOLE( f64Delay );

	if( m_u32OutputFlags & PerfStatsOutput::SCREEN )
		logPerfoamanceCheck_SCREEN( f64Delay );
    
#endif
}

void PerformanceCheck::SetCallDelay( uint32 uCallDelay )
{
	m_u32CallDelay = uCallDelay;
}

void PerformanceCheck::SetEnabled( bool bEnabled )
{
	m_bEnable = bEnabled;
}

void PerformanceCheck::SetOutputFlags( uint32 uFlags )
{
	m_u32OutputFlags = uFlags;
}

bool PerformanceCheck::HasUpdates()
{
	 return m_u32CallDelay == 0 || m_u32CallCount >= m_u32CallDelay;
}

void PerformanceCheck::logPerformanceCheck_CONSOLE( double f64Delay )
{
	std::stringstream ss;
	ss.precision( 5 );
	ss << m_szMessage << " duration: " << f64Delay;

	LOG( ss.str() );
}

void PerformanceCheck::logPerfoamanceCheck_SCREEN( double f64Delay )
{
	StatsManager::GetInstance().AddGuiLineValue( m_szMessage, f64Delay );
}



