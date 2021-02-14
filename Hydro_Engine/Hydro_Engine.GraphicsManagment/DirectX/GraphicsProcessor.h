#pragma once
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsProcessor
{
public:
	GraphicsProcessor();
	~GraphicsProcessor();
	// Initalize D3D12
	bool Initialize(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen);
	// ToDo: Add support for rendering a list of objects. Hash on layer?
	void Render();
	// Clean up
	void Shutdown();
private:
	bool mVSyncEnabled;
	ID3D12Device* mDevice;
	ID3D12CommandQueue* mCommandQueue;
	char mVideoCardDescription[128];
	IDXGISwapChain3* mSwapChain;
	ID3D12DescriptorHeap* mRenderTargetViewHeap;
	ID3D12Resource* mBackBufferRenderTarget[2];
	unsigned int mBufferIndex;
	ID3D12CommandAllocator* mCommandAllocator;
	ID3D12GraphicsCommandList* mCommandList;
	ID3D12PipelineState* mPipelineState;
	ID3D12Fence* mFence;
	HANDLE mFenceEvent;
	unsigned long mFenceValue;
	int mVideoCardMemory;
};

