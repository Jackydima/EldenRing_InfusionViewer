#pragma once

#include <Windows.h>
#include <MinHook.h>
#include <Commctrl.h>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "libMinHook.x64.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "../tools/debug_print.h"


struct FrameContext {
    ID3D12CommandAllocator* CommandAllocator;
    ID3D12Resource* RenderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE RTV;
};

bool InitMenu(bool* a_RunningParam);
bool CleanUpMenu();