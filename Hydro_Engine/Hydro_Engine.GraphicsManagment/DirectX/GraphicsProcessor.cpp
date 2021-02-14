#include "GraphicsProcessor.h"

/////////////////////////////////
//This will be rewritten at a later date. 
//This class will currently start DirectX
//
/////////////////////////////////


GraphicsProcessor::GraphicsProcessor()
{
	mDevice = 0;
	mCommandQueue = 0;
	mSwapChain = 0;
	mRenderTargetViewHeap = 0;
	mBackBufferRenderTarget[0] = 0;
	mBackBufferRenderTarget[1] = 0;
	mCommandAllocator = 0;
	mCommandList = 0;
	mPipelineState = 0;
	mFence = 0;
	mFenceEvent = 0;
}


GraphicsProcessor::~GraphicsProcessor()
{
}


// Initalize D3D12
bool GraphicsProcessor::Initialize(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	IDXGIFactory4* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, renderTargetViewDescriptorSize = 1;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	IDXGISwapChain* swapChain;
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;

	mVSyncEnabled = vsync;
	HRESULT result = D3D12CreateDevice(NULL, featureLevel, __uuidof(ID3D12Device), (void**)&mDevice);

	if (FAILED(result))
	{
		MessageBox(hwnd, L"Could not create a DirectX 12.1 device.  The default video card does not support DirectX 12.1.", L"DirectX Device Failure", MB_OK);
		return false;
	}

	ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	result = mDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&mCommandQueue);

	if (FAILED(result))
		return false;

	result = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);
	if (FAILED(result))
		return false;

	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
		return false;

	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
		return false;

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
		return false;

	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
		return false;

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
		return false;

	for (i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Height == (unsigned int)screenHeight)
		{
			if (displayModeList[i].Width == (unsigned int)screenWidth)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
		return false;

	mVideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	int error = wcstombs_s(&stringLength, mVideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
		return false;

	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;

	adapter->Release();
	adapter = 0;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hwnd;

	if (fullscreen)
		swapChainDesc.Windowed = false;
	else
		swapChainDesc.Windowed = true;

	if (mVSyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	result = factory->CreateSwapChain(mCommandQueue, &swapChainDesc, &swapChain);
	if (FAILED(result))
		return false;

	result = swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&mSwapChain);
	if (FAILED(result))
		return false;

	swapChain = 0;
	factory->Release();
	factory = 0;

	ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));
	renderTargetViewHeapDesc.NumDescriptors = 2;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = mDevice->CreateDescriptorHeap(&renderTargetViewHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&mRenderTargetViewHeap);
	if (FAILED(result))
		return false;

	renderTargetViewHandle = mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	result = mSwapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&mBackBufferRenderTarget[0]);
	if (FAILED(result))
		return false;

	mDevice->CreateRenderTargetView(mBackBufferRenderTarget[0], NULL, renderTargetViewHandle);
	renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	result = mSwapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&mBackBufferRenderTarget[1]);
	if (FAILED(result))
		return false;

	mDevice->CreateRenderTargetView(mBackBufferRenderTarget[1], NULL, renderTargetViewHandle);
	mBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	result = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&mCommandAllocator);
	if (FAILED(result))
		return false;

	result = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&mCommandList);
	if (FAILED(result))
		return false;

	result = mCommandList->Close();
	if (FAILED(result))
		return false;

	result = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&mFence);
	if (FAILED(result))
		return false;

	mFenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (mFenceEvent == NULL)
		return false;

	mFenceValue = 1;

	return true;
}


// ToDo: Add support for rendering a list of objects. Hash on layer?
void GraphicsProcessor::Render()
{
	HRESULT result;
	D3D12_RESOURCE_BARRIER barrier;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	unsigned int renderTargetViewDescriptorSize;
	float color[4];
	ID3D12CommandList* ppCommandLists[1];
	unsigned long long fenceToWaitFor;

	result = mCommandAllocator->Reset();
	if (FAILED(result))
		return;

	result = mCommandList->Reset(mCommandAllocator, mPipelineState);
	if (FAILED(result))
		return;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mBackBufferRenderTarget[mBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	mCommandList->ResourceBarrier(1, &barrier);
	renderTargetViewHandle = mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (mBufferIndex == 1)
	{
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	}

	mCommandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, NULL);
	color[0] = 0.5;
	color[1] = 0.5;
	color[2] = 0.5;
	color[3] = 1.0;
	mCommandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, NULL);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	mCommandList->ResourceBarrier(1, &barrier);
	result = mCommandList->Close();
	if (FAILED(result))
		return;

	ppCommandLists[0] = mCommandList;
	mCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	if (mVSyncEnabled)
	{
		result = mSwapChain->Present(1, 0);
		if (FAILED(result))
			return;
	}
	else
	{
		result = mSwapChain->Present(0, 0);
		if (FAILED(result))
			return;
	}

	fenceToWaitFor = mFenceValue;
	result = mCommandQueue->Signal(mFence, fenceToWaitFor);
	if (FAILED(result))
		return;

	mFenceValue++;

	if (mFence->GetCompletedValue() < fenceToWaitFor)
	{
		result = mFence->SetEventOnCompletion(fenceToWaitFor, mFenceEvent);
		if (FAILED(result))
			return;

		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mBufferIndex == 0 ? mBufferIndex = 1 : mBufferIndex = 0;
}


// Clean up
void GraphicsProcessor::Shutdown()
{
	if (mSwapChain)
		mSwapChain->SetFullscreenState(false, NULL);

	CloseHandle(mFenceEvent);

	if (mFence)
	{
		mFence->Release();
		mFence = 0;
	}

	if (mPipelineState)
	{
		mPipelineState->Release();
		mPipelineState = 0;
	}

	if (mCommandList)
	{
		mCommandList->Release();
		mCommandList = 0;
	}

	if (mCommandAllocator)
	{
		mCommandAllocator->Release();
		mCommandAllocator = 0;
	}

	if (mBackBufferRenderTarget[0])
	{
		mBackBufferRenderTarget[0]->Release();
		mBackBufferRenderTarget[0] = 0;
	}

	if (mBackBufferRenderTarget[1])
	{
		mBackBufferRenderTarget[1]->Release();
		mBackBufferRenderTarget[1] = 0;
	}

	if (mRenderTargetViewHeap)
	{
		mRenderTargetViewHeap->Release();
		mRenderTargetViewHeap = 0;
	}

	if (mSwapChain)
	{
		mSwapChain->Release();
		mSwapChain = 0;
	}

	if (mCommandQueue)
	{
		mCommandQueue->Release();
		mCommandQueue = 0;
	}

	if (mDevice)
	{
		mDevice->Release();
		mDevice = 0;
	}

	return;
}
