#pragma once

#include <windows.h>
#include <string>
#include "Prerequisites.h"

//TODO: Move to a more general solution (OpenGL/DX) in fancy_core

struct WindowParameters
{
	std::string myTitle;
	uint myWidth;
	uint myHeight;
};

class WindowDX12
{
public:
	using ResizeFunc = void(*) (uint, uint);

	bool init(HINSTANCE anInstanceHandle, int aShowWindowMode, const WindowParameters& someParams);

	WindowDX12();
	~WindowDX12();

	ResizeFunc myResizeCallback;

	HWND myWindowHandle;

private:
	


};

