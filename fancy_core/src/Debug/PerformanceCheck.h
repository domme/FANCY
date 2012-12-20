#ifndef PERFORMANCECHECK_H
#define PERFORMANCECHECK_H

#include "../Includes.h"

namespace PerfStatsOutput
{
	enum EOutputMode
	{
		DEBUG_CONSOLE = 0x00000001,
		SCREEN		  = 0x00000010
	};
}


class DLLEXPORT PerformanceCheck
{
public:
	explicit PerformanceCheck();

	void StartPerformanceCheck( const String& szMessage );
	void FinishAndLogPerformanceCheck();
	static void FrameStart();
	static void SetCallDelay( uint32 uCallDelay );
	static void SetEnabled( bool bEnabled );
	static void SetOutputFlags( uint32 uFlags );
	static bool HasUpdates();

private:
	void logPerformanceCheck_CONSOLE( double f64Delay );
	void logPerfoamanceCheck_SCREEN( double f64Delay );

	uint64 m_u64StartTime;
	String m_szMessage;
	static uint32 m_u32CallCount;
	static uint32 m_u32CallDelay;
	static bool	  m_bEnable;
	static uint32 m_u32OutputFlags;
};


#define CHECK_PERFORMANCE( func, message )			\
{													\
	PerformanceCheck __clPerfCheck;					\
	__clPerfCheck.StartPerformanceCheck( message );	\
	func;											\
	__clPerfCheck.FinishAndLogPerformanceCheck();	\
}


#endif