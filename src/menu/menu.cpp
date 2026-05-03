#include "menu.h"

// Hooking pointer globals
WNDPROC OriginalWndProc = nullptr;

using Present_t = HRESULT(__stdcall*)(IDXGISwapChain* p_This, UINT SyncInterval, UINT Flags);
Present_t OriginalPresent = nullptr;
void* Present_Func = nullptr;

using ResizeBuffer_t = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain* p_This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffer_t OriginalResizeBuffer = nullptr;
void* ResizeBuffers_Func = nullptr;

using GetDeviceState_t = HRESULT(__stdcall*)(IDirectInputDevice8*, DWORD, LPVOID);
GetDeviceState_t OriginalGetDeviceState = nullptr;
void* GetDeviceState_Func = nullptr;

// Unused at the moment
using GetDeviceData_t = HRESULT(__stdcall*)(IDirectInputDevice8*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
GetDeviceData_t OriginalGetDeviceData = nullptr;
void* GetDeviceData_Func = nullptr;

using XInputGetState_t = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);
XInputGetState_t OriginalXInputGetState = nullptr;
void* XInputGetState_Func = nullptr;

using GetRawInputData_t = UINT(WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
GetRawInputData_t OriginalGetRawInputData = nullptr;
void* GetRawInputData_Func = nullptr;

using SetCursorPos_t = BOOL(WINAPI*)(int, int);
SetCursorPos_t OriginalSetCursorPos = nullptr;
void* SetCursorPos_Func;

UINT g_BufferCount = 0;

// Imgui needed globals
HWND g_Hwnd = nullptr;
ID3D11Device* g_Device = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Menu globals
bool g_Initialized = false;
bool g_WndProcHooked = false;
bool g_MenuActive = false;

// Callee DLL Main loop bool
bool* g_pRunning = nullptr;
// Globals END

// Foreward Declarations:
static void RenderMenu();
static void UninitializeImGui();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static void CleanUpDevice();
static DWORD WINAPI CallForCleanUpFunction(LPVOID a_FunctionParam);
static bool StartHooking();
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using CleanupFunction_t = void(__stdcall*)(void);
static DWORD WINAPI CallForCleanUpFunction(LPVOID a_FunctionParam)
{
    CleanupFunction_t CleanUp = reinterpret_cast<CleanupFunction_t>(a_FunctionParam);
    Sleep(config::cycleSpeed + 200);
    CleanUp();
    
    return 0;
}

static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SYSKEYDOWN:
        if (wParam == VK_NUMPAD1 && (GetAsyncKeyState(VK_MENU) < 0))
        {
            g_MenuActive = !g_MenuActive;
            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = g_MenuActive;
            return 0;
        }

        if (wParam == VK_END && (GetAsyncKeyState(VK_MENU) < 0))
        {
            if (g_pRunning != nullptr)
                *g_pRunning = false;
            return 0;
        }
        break;
    }

    if (g_Initialized && g_MenuActive)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
    }

    return CallWindowProcW(OriginalWndProc, hWnd, msg, wParam, lParam);
}

static void RenderMenu()
{
    // Background specific things
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowBgAlpha(0.6f);

    ImGui::Begin("OverlayBG_Window", nullptr, flags);
    ImGui::End();

    // Mod Page Render Objects
    // ImGui::ShowDemoWindow();
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Main Menu @Jackydima");

    if (ImGui::BeginTabBar("Menu Sections"))
    {
        if (ImGui::BeginTabItem("Visual Configuraions"))
        {
            //ImGui::Checkbox("Infusion Viewing Enabled", &config::InfusionViewerActive);

            ImGui::SeparatorText("Info");
            ImGui::Text("To open or close the Menu use the Keys 'ALT + Num1'");
            ImGui::Text("To Deactive and Unload this Mod press 'ALT + End'");

            ImGui::SeparatorText("Info");
            ImGui::Text("Define the Cyclespeed for updating the visual effects!");
            ImGui::DragInt("Cycle-Speed", &config::cycleSpeed, 1.0f, 1, 9999);

            ImGui::SeparatorText("Infusion Viewer:");
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
            if (ImGui::Checkbox("Infusion Viewer Active", &config::InfusionViewerActive))
            {
                if (!config::InfusionViewerActive)
                {
                    //RemoveEffectForPlayers();
                    CreateThread(NULL, 0, &CallForCleanUpFunction, reinterpret_cast<LPVOID>(RemoveEffectForPlayers), 0, nullptr);
                }
            }
            ImGui::PopStyleVar();

            ImGui::SeparatorText("Debug Phantom Color:");
            ImGui::DragInt("SelfPhantomId", &config::PhantomSelfId, 1.0f, -1, 1000);

            ImGui::DragInt("Net1 PhantomId", &config::NetPlayer1Id, 1.0f, -1, 1000);

            ImGui::DragInt("Net2 PhantomId", &config::NetPlayer2Id, 1.0f, -1, 1000);

            ImGui::DragInt("Net3 PhantomId", &config::NetPlayer3Id, 1.0f, -1, 1000);

            ImGui::DragInt("Net4 PhantomId", &config::NetPlayer4Id, 1.0f, -1, 1000);

            ImGui::DragInt("Net5 PhantomId", &config::NetPlayer5Id, 1.0f, -1, 1000);

            if (ImGui::Checkbox("Debug Phantom Coloring", &config::PhantomColorActive))
            {
                if (!config::PhantomColorActive)
                {
                    //DeactivatePhantomColor();
                    CreateThread(NULL,0, &CallForCleanUpFunction, reinterpret_cast<LPVOID>(DeactivatePhantomColor), 0, nullptr);
                }
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

// x64 this call has rcx for std call in the first parameter!
static HRESULT __stdcall Hook_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    g_SwapChain = pSwapChain;
    if (!g_Initialized)
    {
        logger::println("Started Menu Init in Present"); 
        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        g_Hwnd = desc.OutputWindow;

        g_BufferCount = desc.BufferCount;

        if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_Device))))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        g_Device->GetImmediateContext(&g_pd3dDeviceContext);

        CreateRenderTarget();

        ImGui_ImplWin32_Init(g_Hwnd);
        ImGui_ImplDX11_Init(g_Device, g_pd3dDeviceContext);

        if (!g_WndProcHooked)
        {
            OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HookWndProc)));
            g_WndProcHooked = true;
        }

        g_Initialized = true;
    }

    if (!g_MenuActive)
        return OriginalPresent(pSwapChain, SyncInterval, Flags);

    // ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RenderMenu();

    ImGui::Render();

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    auto hr = OriginalPresent(pSwapChain, SyncInterval, Flags);
    if (hr != S_OK)
    {
        logger::println("Issue with Present: %x", hr);
    }
    return hr;
}

static HRESULT STDMETHODCALLTYPE Hook_ResizeBuffer(IDXGISwapChain* p_This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (g_Initialized)
    {
        CleanupRenderTarget();
    }
    // Continue with Original
    auto hr = OriginalResizeBuffer(p_This, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (g_Initialized)
    {
        CreateRenderTarget();
    }

    return hr;
}

// Mouse Input
static HRESULT __stdcall Hook_GetDeviceState(IDirectInputDevice8* a_pThis, DWORD cbData, LPVOID lpvData)
{
    HRESULT result = OriginalGetDeviceState(a_pThis, cbData, lpvData);

    if (!g_MenuActive)
        return result;

    // Keyboard input probably
    if (cbData == 256)
    {
        for (int i = 0; i < 256; i++)
        {
            ((BYTE*)lpvData)[i] = 0x00; // reset keyboard state!
        }
    }

    // We probably have DIMOUSESTATE information here
    if (cbData == sizeof(DIMOUSESTATE))
    {
        ((LPDIMOUSESTATE)lpvData)->lX = 0;
        ((LPDIMOUSESTATE)lpvData)->lZ = 0;
        ((LPDIMOUSESTATE)lpvData)->lY = 0;
        ((LPDIMOUSESTATE)lpvData)->rgbButtons[0] = 0;
        ((LPDIMOUSESTATE)lpvData)->rgbButtons[1] = 0;
    }

    // We probably have DIMOUSESTATE2 information here
    if (cbData == sizeof(DIMOUSESTATE2))
    {
        ((LPDIMOUSESTATE2)lpvData)->lX = 0;
        ((LPDIMOUSESTATE2)lpvData)->lZ = 0;
        ((LPDIMOUSESTATE2)lpvData)->lY = 0;
        ((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] = 0;
        ((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] = 0;
    }

    return result;
}

static HRESULT __stdcall Hook_GetDeviceData(IDirectInputDevice8* a_This, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    HRESULT result = OriginalGetDeviceData(a_This, cbObjectData, rgdod, pdwInOut, dwFlags);

    if (g_MenuActive)
        *pdwInOut = 0;

    return result;
}

// Controller Input
static DWORD WINAPI Hook_XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
    auto result = OriginalXInputGetState(dwUserIndex, pState);
    if (g_MenuActive)
    {
        pState->Gamepad.bLeftTrigger = 0;
        pState->Gamepad.bRightTrigger = 0;
        pState->Gamepad.sThumbLX = 0;
        pState->Gamepad.sThumbLY = 0;
        pState->Gamepad.sThumbRX = 0;
        pState->Gamepad.sThumbRY = 0;
        pState->Gamepad.wButtons = 0;
    }

    return result;
}

// Keyboard Inputs
static UINT WINAPI Hook_GetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
    UINT result = OriginalGetRawInputData(
        hRawInput, uiCommand, pData, pcbSize, cbSizeHeader
    );

    if (!g_MenuActive)
        return result;

    RAWINPUT* raw = (RAWINPUT*)pData;
    if (uiCommand == RID_INPUT && pData)
    {
        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            raw->data.mouse.lLastX = 0;
            raw->data.mouse.lLastY = 0;
        }

        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            raw->data.keyboard.VKey = 0;
            raw->data.keyboard.Flags = RI_KEY_BREAK; // mark as released
        }
    }

    return result;
}

// Free Mouse Movement
BOOL WINAPI Hook_SetCursorPos(int X, int Y)
{
    if (g_MenuActive)
    {
        return TRUE;
    }

    return OriginalSetCursorPos(X, Y);
}

static bool InitGameMenuPointers(HMODULE a_Module)
{
    // DXGI d3d12 pointers
    // Create dummy window
    HWND hwnd = CreateWindowA("STATIC", "dummy", WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);

    DXGI_SWAP_CHAIN_DESC scdesc = {};
    scdesc.BufferCount = 2;
    scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scdesc.OutputWindow = hwnd;
    scdesc.SampleDesc.Count = 1;
    scdesc.Windowed = TRUE;
    scdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    IDXGISwapChain* swapchain = nullptr;
    ID3D11Device* device = nullptr;
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &scdesc,
        &swapchain,
        &device,
        &featureLevel,
        &g_pd3dDeviceContext);
    
    if (FAILED(res))
    {
        return false;
    }

    {
        void** vtable = *reinterpret_cast<void***>(swapchain);
        Present_Func = vtable[8];
        ResizeBuffers_Func = vtable[13];
    }


    // Cleanup
    swapchain->Release();
    device->Release();
    
    DestroyWindow(hwnd);


    // DInput pointers
    IDirectInput8* pDirectInput;
    if (DirectInput8Create(a_Module, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<LPVOID*>(&pDirectInput), NULL) != DI_OK)
    {
        logger::println("Could not create DirectInput8");
        return false;
    }

    IDirectInputDevice8* pDIMouse;
    if (pDirectInput->CreateDevice(GUID_SysMouse, &pDIMouse, NULL) != DI_OK)
    {
        pDirectInput->Release();
        logger::println("Could not create DirectInput8 Mouse Device");
        return false;
    }

    {
        void** vtable = *reinterpret_cast<void***>(pDIMouse);
        GetDeviceState_Func = vtable[9];
        GetDeviceData_Func = vtable[10];
    }

    // Cleanup
    pDIMouse->Release();
    pDirectInput->Release();

    // XInput Pointers
    HMODULE xinput1_3Handle = GetModuleHandleA("xinput1_3.dll");
    if (xinput1_3Handle == NULL)
    {
        logger::println("No xinput1_3.dll loaded");
        return false;
    }

    XInputGetState_Func = GetProcAddress(xinput1_3Handle, "XInputGetState");
    if (!XInputGetState_Func)
    {
        logger::println("XInputGetState not found");
        return false;
    }

    HMODULE user32Handle = GetModuleHandleA("user32.dll");
    if (user32Handle == NULL)
    {
        logger::println("user32 Handle not found");
        return false;
    }
    GetRawInputData_Func = GetProcAddress(user32Handle, "GetRawInputData");
    if (!GetRawInputData_Func)
    {
        logger::println("GetRawInputData not found");
        return false;
    }


    SetCursorPos_Func = GetProcAddress(user32Handle, "SetCursorPos");
    if (!SetCursorPos_Func)
    {
        logger::println("GetRawInputData not found");
        return false;
    }

    return true;
}

static bool StartHooking()
{
    if (!Present_Func)
    {
        logger::println("Getting Present vtable function failed!");
        return false;
    }

    if (MH_CreateHook(Present_Func, &Hook_Present, (LPVOID*)&OriginalPresent) != MH_OK)
    {
        logger::println("MH: Hook Present failed");
        return false;
    }

    if (!ResizeBuffers_Func)
    {
        logger::println("Getting ResizeBuffers vtable function failed!");
        return false;
    }

    if (MH_CreateHook(ResizeBuffers_Func, &Hook_ResizeBuffer, (LPVOID*)&OriginalResizeBuffer) != MH_OK)
    {
        logger::println("MH: Hook ResizeBuffers failed");
        return false;
    }

    //GetDeviceData_t getDeviceData = reinterpret_cast<GetDeviceData_t>(GetDeviceDataPtr);
    if (!GetDeviceData_Func)
    {
        logger::println("Getting GetDeviceData vtable function failed!");
        return false;
    }

    if (MH_CreateHook(GetDeviceData_Func, &Hook_GetDeviceData, (LPVOID*)&OriginalGetDeviceData) != MH_OK)
    {
        logger::println("MH: Hook GetDeviceData failed");
        return false;
    }

    //GetDeviceState_t getDeviceState = reinterpret_cast<GetDeviceState_t>(GetDeviceState_Func);
    if (!GetDeviceState_Func)
    {
        logger::println("Getting GetDeviceState vtable function failed!");
        return false;
    }

    if (MH_CreateHook(GetDeviceState_Func, &Hook_GetDeviceState, (LPVOID*)&OriginalGetDeviceState) != MH_OK)
    {
        logger::println("MH: Hook GetDeviceState failed");
        return false;
    }

    if (!XInputGetState_Func)
    {
        logger::println("Getting XInputGetState function failed!");
        return false;
    }
    if (MH_CreateHook(XInputGetState_Func, &Hook_XInputGetState, (LPVOID*)&OriginalXInputGetState) != MH_OK)
    {
        logger::println("MH: Hook XInputGetState failed");
        return false;
    }

    //GetRawInputData_t getRawInputData = reinterpret_cast<GetRawInputData_t>(GetRawInputData_Func);
    if (!GetRawInputData_Func)
    {
        logger::println("Getting GetRawInputData function failed!");
        return false;
    }
    if (MH_CreateHook(GetRawInputData_Func, &Hook_GetRawInputData, (LPVOID*)&OriginalGetRawInputData) != MH_OK)
    {
        logger::println("MH: Hook GetRawInputData failed");
        return false;
    }

    if (!SetCursorPos_Func)
    {
        logger::println("Getting SetCursorPos function failed!");
        return false;
    }
    if (MH_CreateHook(SetCursorPos_Func, &Hook_SetCursorPos, (LPVOID*)&OriginalSetCursorPos) != MH_OK)
    {
        logger::println("MH: Hook SetCursorPos_Func failed");
        return false;
    }


    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        logger::println("MH: Enabling Hooking Functions failed");
        return false;
    }

    return true;
}

bool InitMenu(HMODULE a_Module, bool* a_pRunningParam, int a_Delay)
{
    g_pRunning = a_pRunningParam;

    Sleep(a_Delay); // Wait for other GUI related stuff to load/hook - e.x. Steam Overlay!
    
    if (!InitGameMenuPointers(a_Module))
        return false;

    if (!StartHooking())
        return false;

    logger::println("Address of Present_Func: %p", Present_Func);
    logger::println("Address of ResizeBuffers_Func: %p", ResizeBuffers_Func);
    logger::println("Address of GetDeviceState_Func: %p", GetDeviceState_Func);
    logger::println("Address of GetRawInputData_Func: %p", GetRawInputData_Func);

    return true;
}

static void UninitializeImGui()
{
    if (g_Initialized)
    {
        g_Initialized = false; // force re-init

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanUpDevice();
    }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    if (FAILED(g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
    {
        logger::println("Failed to create RenderTargetView");
        return;
    }
    g_Device->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

static void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

static void CleanUpDevice()
{
    CleanupRenderTarget();
    g_Device = nullptr;
    g_pd3dDeviceContext = nullptr;
}

bool CleanUpMenu()
{
    if (g_WndProcHooked)
    {
        SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OriginalWndProc));
        g_WndProcHooked = false;
    }

    UninitializeImGui();

    return true;;
}