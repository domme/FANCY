#ifndef STATSGUI_H
#define STATSGUI_H

#include "../Includes.h"

struct SStatsEntry
{
	std::string		m_szMessage;
	double			m_f64Value;
};

class DLLEXPORT StatsManager
{
public:
	static StatsManager& GetInstance();
	~StatsManager(void);
	
	void AddGuiLineValue( const String& szMessage, float fValue );
	void AddGuiLineValue( const String& szMessage, double fValue );
	void Clear()															{ m_vStats.clear(); }
	void RenderGuiLinesGLUT();
	void SetScreenPosition( const glm::vec2& v2ScreenPos )					{ m_v2ScreenPosition = v2ScreenPos; }
	void SetViewportSize( const glm::vec2& v2ViewportSize )					{ m_v2ViewportSize = v2ViewportSize; }
	const std::vector<SStatsEntry>& GetStats()								{ return m_vStats; }

private:
	StatsManager(void);

	std::vector<SStatsEntry> m_vStats;
	glm::vec2			m_v2ScreenPosition;
	glm::vec2			m_v2ViewportSize;

};

#endif

