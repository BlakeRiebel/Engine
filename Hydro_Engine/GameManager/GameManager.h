#pragma once
#include <Windows.h>
#include <iostream>
#include "../Hydro_Engine.GraphicsManagment/DirectX/GraphicsProcessor.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GameManager
{
private:
	GraphicsProcessor mGraphics;
public:
	GameManager();
	~GameManager();
	HRESULT Load(int screenHeight, int screenWidth, HWND hwnd);
	HRESULT Run();
	HRESULT Shutdown();
};

