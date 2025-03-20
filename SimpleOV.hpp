#pragma once
#include <Windows.h>
#include <dwmapi.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_internal.h"
#include <d3d11.h>
#include <thread>
#include <chrono>

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace overlay
{
    // Variables
    HWND Window;
    WNDCLASSEXA wcex;

    static ULONG G_Width = GetSystemMetrics(SM_CXSCREEN);
    static ULONG G_Height = GetSystemMetrics(SM_CYSCREEN);

    static ID3D11Device* g_pd3dDevice = nullptr;
    static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    static IDXGISwapChain* g_pSwapChain = nullptr;
    static bool g_SwapChainOccluded = false;
    static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
    static bool ShouldQuit;
    static HANDLE waitableObject = nullptr;
    static bool vsync = true;

    VOID CreateRenderTarget();
    VOID SetupWindow();
    bool CreateDeviceD3D(HWND hWnd);
    VOID CleanupDeviceD3D();
    VOID CloseOverlay();
    VOID Render();
    VOID EndRender();
}

void overlay::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

bool overlay::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) {
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    }
    if (FAILED(res)) {
        return false;
    }

    overlay::CreateRenderTarget();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; 
    ImGui_ImplWin32_Init(overlay::Window);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    SetWindowPos(overlay::Window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    return true;
}

void overlay::SetupWindow()
{
    overlay::wcex = {
        sizeof(WNDCLASSEXA),
        0,
        DefWindowProcA,
        0,
        0,
        nullptr,
        LoadIcon(nullptr, IDI_APPLICATION),
        LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        "ovh",
        nullptr
    };

    RECT Rect;
    GetWindowRect(GetDesktopWindow(), &Rect);

    RegisterClassExA(&wcex);

    overlay::Window = CreateWindowExA(NULL, "ovh", "ovh", WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
    SetWindowLong(overlay::Window, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
    SetLayeredWindowAttributes(overlay::Window, RGB(0, 0, 0), 255, NULL);

    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(overlay::Window, &margin);

    ShowWindow(overlay::Window, SW_SHOW);
    UpdateWindow(overlay::Window);
}

void overlay::CleanupDeviceD3D()
{
    if (overlay::g_mainRenderTargetView) { overlay::g_mainRenderTargetView->Release(); overlay::g_mainRenderTargetView = nullptr; }
    if (overlay::g_pSwapChain) { overlay::g_pSwapChain->Release(); overlay::g_pSwapChain = nullptr; }
    if (overlay::g_pd3dDeviceContext) { overlay::g_pd3dDeviceContext->Release(); overlay::g_pd3dDeviceContext = nullptr; }
    if (overlay::g_pd3dDevice) { overlay::g_pd3dDevice->Release(); overlay::g_pd3dDevice = nullptr; }
}

void overlay::CloseOverlay()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(overlay::Window);
    ::UnregisterClassA(overlay::wcex.lpszClassName, overlay::wcex.hInstance);
}

VOID overlay::Render()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            overlay::ShouldQuit = true;
        }
    }
    if (overlay::ShouldQuit) {
        overlay::CloseOverlay();
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    POINT p;
    GetCursorPos(&p);
    io.MousePos.x = static_cast<float>(p.x);
    io.MousePos.y = static_cast<float>(p.y);

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        io.MouseDown[0] = true;
        if (!io.MouseDown[0]) { 
            io.MouseClicked[0] = true;
            io.MouseClickedPos[0].x = io.MousePos.x;
            io.MouseClickedPos[0].y = io.MousePos.y;
        }
    }
    else {
        io.MouseDown[0] = false;
        io.MouseClicked[0] = false; 
    }

    overlay::g_SwapChainOccluded = false;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

VOID overlay::EndRender()
{
    ImGui::Render();
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);
}