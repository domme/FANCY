#include "WindowDX12.h"

// Main message handler for the sample.
LRESULT CALLBACK locOnWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle destroy/shutdown messages.
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}


WindowDX12::WindowDX12() :
	myWindowHandle(nullptr)
{

}

WindowDX12::~WindowDX12()
{


}


bool WindowDX12::init(HINSTANCE anInstanceHandle, int aShowWndowMode, const WindowParameters& someParams)
{
	if (myWindowHandle)
		return true;

	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = locOnWindowEvent;
	windowClass.hInstance = anInstanceHandle;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "WindowClass1";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(someParams.myWidth), static_cast<LONG>(someParams.myHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	myWindowHandle = CreateWindowEx(NULL,
		"WindowClass1",
		"Fancy (DX12)",
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,		// We have no parent window, NULL.
		NULL,		// We aren't using menus, NULL.
		anInstanceHandle,
		NULL);		// We aren't using multiple windows, NULL.

	ShowWindow(myWindowHandle, aShowWndowMode);

	return true;
}


