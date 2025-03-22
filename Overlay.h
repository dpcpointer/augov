#pragma once
#ifndef SIMPLEOV_HPP
#define SIMPLEOV_HPP

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
    extern __declspec(dllexport) DWORD NvOptimusEnablement;
    extern __declspec(dllexport) int AmdPowerXpressRequestHighPerformance;
}

namespace overlay {
    extern HWND Window;
    extern WNDCLASSEXA wcex;
    extern ULONG G_Width;
    extern ULONG G_Height;
    extern ID3D11Device* g_pd3dDevice;
    extern ID3D11DeviceContext* g_pd3dDeviceContext;
    extern IDXGISwapChain* g_pSwapChain;
    extern bool g_SwapChainOccluded;
    extern ID3D11RenderTargetView* g_mainRenderTargetView;
    extern bool ShouldQuit;
    extern HANDLE waitableObject;
    extern bool vsync;

    // Function declarations
    void CreateRenderTarget();
    void SetupWindow();
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CloseOverlay();
    void Render();
    void EndRender();
    bool SetupOverlay();
}

#endif // SIMPLEOV_HPP