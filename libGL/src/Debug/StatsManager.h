#ifndef STATSGUI_H
#define STATSGUI_H

#include "../Includes.h"

class DLLEXPORT StatsManager
{
public:
	static StatsManager& GetInstance();
	~StatsManager(void);
	
	void AddGuiLineValue( const String& szMessage, float fValue );
	void AddGuiLineValue( const String& szMessage, double fValue );
	void Clear()															{ m_vGuiLines.clear(); }
	void RenderGuiLinesGLUT();
	void SetScreenPosition( const glm::vec2& v2ScreenPos )					{ m_v2ScreenPosition = v2ScreenPos; }
	void SetViewportSize( const glm::vec2& v2ViewportSize )					{ m_v2ViewportSize = v2ViewportSize; }
	const std::vector<String>& GetStatsLines()								{ return m_vGuiLines; }

private:
	StatsManager(void);

	std::vector<String> m_vGuiLines;
	glm::vec2			m_v2ScreenPosition;
	glm::vec2			m_v2ViewportSize;

};

#endif

