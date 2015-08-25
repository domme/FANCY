#include <windows.h>

#include <EngineCommon.h>

#include "Prerequisites.h"
#include "WindowDX12.h"
#include "RendererDX12.h"

WindowDX12 ourWindow;
RendererDX12 ourRenderer;
constexpr uint kWidth = 1280u;
constexpr uint kHeight = 720u;

void onWindowResize(uint aWidth, uint aHeight)
{
	
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	WindowParameters params;
	params.myTitle = "Fancy Engine (DX12)";
	params.myWidth = kWidth;
	params.myHeight = kHeight;

	if (!ourWindow.init(hInstance, nCmdShow, params))
		return -1;

	ourWindow.myResizeCallback = onWindowResize;

	ourRenderer.init(kWidth, kHeight, &ourWindow.myWindowHandle);
	ourRenderer.postInit();

	MSG msg = { 0 };
	while (true)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;

		}

		ourRenderer.beginFrame();
		ourRenderer.render();
		ourRenderer.endFrame();
	}

	return 0;
}