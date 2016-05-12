#include "mainwindow.h"
#include "image.h"

#pragma comment(lib, "Winmm.lib")

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	SetCurrentDirectory("../terrain");
	MainWindow wnd;
	wnd.Show(SW_SHOW);

	const double MS_PER_FRAME = 16;

	timeBeginPeriod(1);

	MSG msg = { };
	int lastTime = timeGetTime();
	while (msg.message != WM_QUIT)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) return 0;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		int curTime = timeGetTime();
		int dt = curTime - lastTime;
		
		if (wnd.IsVsyncEnabled() || dt >= MS_PER_FRAME)
		{
			wnd.Update((int)dt);
			wnd.Redraw();
			lastTime = curTime;
		}
		//Sleep(1);
	}

	timeEndPeriod(1);

	return 0;
}
