#include "GameManager\GameManager.h"
#include <time.h>
uint32_t RenderWidth;
uint32_t RenderHeight;
unsigned int Width = /*1024;*/ 1080;
unsigned int Height = /*720;*/ 720;
using namespace std;



WNDCLASSEX Win;
HINSTANCE m_hinstance;
HWND m_hwnd;
LPCWSTR m_applicationName;
GameManager Game;

bool Init(HINSTANCE hInstance);


int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	srand((unsigned int)time(0));


	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();

	Init(hInstance);

	MSG msg = { 0 };

	char buff[100];
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			msg.message = WM_QUIT;
			Game.Shutdown();
		}
		else
		{
			Game.Run();
		}
	}

	return static_cast<int>(msg.wParam);
}
LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (true)
		return MsgProc(hwnd, msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Init(HINSTANCE hInstance)
{
	m_hinstance = hInstance;
	m_hwnd = NULL;
	m_applicationName = L"Blood City";

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_hinstance;
	wcex.lpfnWndProc = MainWndProc;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"DXAPPWNDCLASS";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		OutputDebugString(L"FAILED TO CREATE WINDOW CLASS");
		return false;
	}

	RECT r = { 0, 0, (LONG)Width, (LONG)Height };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
	UINT width = r.right - r.left;
	UINT height = r.bottom - r.top;

	UINT x = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
	UINT y = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;

	m_hwnd = CreateWindow(L"DXAPPWNDCLASS", L"Blood City", WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, m_hinstance, NULL);

	if (!m_hwnd)
	{
		OutputDebugString(L"FAILED TO CREATE WINDOW");
		return false;
	}

	ShowWindow(m_hwnd, SW_SHOW);

	Game = GameManager();

	Game.Load(GetSystemMetrics(SM_CYSCREEN), GetSystemMetrics(SM_CXSCREEN), m_hwnd);

	return true;

}