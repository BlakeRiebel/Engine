#include "GameManager.h"


GameManager::GameManager()
{
}


GameManager::~GameManager()
{
}


HRESULT GameManager::Load(int screenHeight, int screenWidth, HWND hwnd)
{
	mGraphics = GraphicsProcessor();
	HRESULT result = mGraphics.Initialize(screenHeight, screenWidth, hwnd, VSYNC_ENABLED, FULL_SCREEN);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return result;
	}
	return S_OK;
}


HRESULT GameManager::Run()
{
	try
	{
		mGraphics.Render();
	}
	catch (const std::exception&)
	{
		return E_FAIL;
	}
	return S_OK;
}


HRESULT GameManager::Shutdown()
{
	mGraphics.Shutdown();
	return S_OK;
}
