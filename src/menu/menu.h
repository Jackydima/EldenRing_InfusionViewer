#pragma once

#include <Windows.h>
#include <MinHook.h>
#include <Commctrl.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <d3d11.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <Xinput.h>

#pragma comment(lib, "libMinHook.x64.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")
#pragma comment(lib, "Xinput.lib")

#include "../tools/debug_print.h"
#include "../shared/config.h"
#include "../shared/scripts/scripts.h"

bool InitMenu(HMODULE a_Module, bool* a_RunningParam, int a_Delay);
bool CleanUpMenu();