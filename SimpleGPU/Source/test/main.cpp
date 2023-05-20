#include "api.h"
#include <memory>
#include <Windows.h>
#include <iostream>

LRESULT CALLBACK WindowProcedure(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_DESTROY:
            std::cout << "\ndestroying window\n";
            PostQuitMessage(0);
            return 0L;
        case WM_LBUTTONDOWN:
            std::cout << "\nmouse left button down at (" << LOWORD(lp) << ',' << HIWORD(lp) << ")\n";
        default:
            return DefWindowProc(window, msg, wp, lp);
    }
}

HWND CreateWin32Window()
{
    // Register the window class.
    auto myclass = L"myclass";
    WNDCLASSEX wndclass = {
        sizeof(WNDCLASSEX), CS_DBLCLKS,
        WindowProcedure,
        0, 0, GetModuleHandle(0), LoadIcon(0, IDI_APPLICATION),
        LoadCursor(0, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1),
        0, myclass, LoadIcon(0, IDI_APPLICATION)
    };
    static bool bRegistered = RegisterClassEx(&wndclass);
    if (bRegistered)
    {
        HWND window = CreateWindowEx(0, myclass, TEXT("title"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
        if (window)
        {
            ShowWindow(window, SW_SHOWDEFAULT);
        }
        return window;
    }
    return nullptr;
}

int main()
{
	GPUInstanceDescriptor desc
	{ 
		.pChained = nullptr,
		.backend = EGPUBackend::GPUBackend_Vulkan,
		.enableDebugLayer = true,
		.enableValidation = true
	};
	GPUInstanceID pInstance = GPUCreateInstance(&desc);
    uint32_t adapterCount = 0;
    GPUEnumerateAdapters(pInstance, NULL, &adapterCount);
    DECLEAR_ZERO_VAL(GPUAdapterID, adapters, adapterCount);
    GPUEnumerateAdapters(pInstance, adapters, &adapterCount);

    auto window = CreateWin32Window();
    GPUSurfaceID pSurface = GPUCreateSurfaceFromNativeView(pInstance, window);

    GPUQueueGroupDescriptor G = {
        .queueType = EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    GPUDeviceDescriptor deviceDesc = {
        .pQueueGroup = &G,
        .queueGroupCount = 1,
        .disablePipelineCache = false
    };
    GPUDeviceID device = GPUCreateDevice(adapters[0], &deviceDesc);

    GPUQueueID pGraphicQueue = GPUGetQueue(device, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);

    GPUSwapchainDescriptor swapchainDesc{};
    swapchainDesc.ppPresentQueues = &pGraphicQueue;
    swapchainDesc.presentQueuesCount = 1;
    swapchainDesc.pSurface = pSurface;
    swapchainDesc.format = EGPUFormat::GPU_FORMAT_B8G8R8A8_SRGB;
    swapchainDesc.width = 1280;
    swapchainDesc.height = 720;
    swapchainDesc.imageCount = 3;
    swapchainDesc.enableVSync = true;
    GPUSwapchainID pSwapchain = GPUCreateSwapchain(device, &swapchainDesc);
    GPUFreeSwapchain(pSwapchain);
    GPUFreeDevice(device);
    GPUFreeSurface(pInstance, pSurface);
	GPUFreeInstance(pInstance);
	return 0;
}