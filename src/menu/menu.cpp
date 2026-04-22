#include "menu.h"

//const int ID_HOTKEY_MENU_TOGGLE = 2000;
//const int ID_HOTKEY_EXIT = 2001;

//ImVec4 clear_color = ImVec4(0.05f, 0.55f, 0.60f, 0.01f);

// Globals

// Hooking pointer globals
WNDPROC OriginalWndProc = nullptr;

using Present_t = HRESULT(__stdcall*)(IDXGISwapChain* p_This, UINT SyncInterval, UINT Flags);
Present_t OriginalPresent = nullptr;
void* Present_Func = nullptr;

using ResizeBuffer_t = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain* p_This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffer_t OriginalResizeBuffer = nullptr;
void* ResizeBuffers_Func = nullptr;

using ExecuteCommandLists_t = void(__stdcall*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const);
ExecuteCommandLists_t OrginalExecuteCommandLists = nullptr;
void* ExecuteCommandLists_Func = nullptr;

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

FrameContext* g_FrameContext = nullptr;
UINT g_BufferCount = 0;

// Imgui needed globals
HWND g_Hwnd = nullptr;
IDXGISwapChain3* g_Swapchain3 = nullptr;
ID3D12Device* g_Device = nullptr;
ID3D12DescriptorHeap* g_RTVHeap = nullptr;
ID3D12DescriptorHeap* g_SrvHeap = nullptr;
ID3D12CommandQueue* g_CommandQueue = nullptr;
ID3D12GraphicsCommandList* g_CmdList = nullptr;

static ID3D12Fence* g_fence = nullptr;
static HANDLE g_fenceEvent = nullptr;
static UINT64 g_fenceLastSignaledValue = 0;

// Menu globals
bool g_Initialized = false;
bool g_WndProcHooked = false;
bool g_MenuActive = false;

// Callee DLL Main loop bool
bool* g_pRunning = nullptr;
// Globals END

// Foreward Declarations:
static void RenderMenu();
void UninitializeImGui();
void WaitForPendingOperations();
static DWORD WINAPI CallForCleanUpFunction(LPVOID a_FunctionParam);
static bool StartHooking();
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using CleanupFunction_t = void(__stdcall*)(void);
static DWORD WINAPI CallForCleanUpFunction(LPVOID a_FunctionParam)
{
    CleanupFunction_t CleanUp = reinterpret_cast<CleanupFunction_t>(a_FunctionParam);
    Sleep(500);
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
    if (!g_Initialized)
    {
        IMGUI_CHECKVERSION();

        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
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

        if (!g_CommandQueue)
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        // Allocate frame contexts
        g_FrameContext = new FrameContext[g_BufferCount];

        // Create a temporary allocator for command list creation
        ID3D12CommandAllocator* tempAllocator = nullptr;
        if (FAILED(g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tempAllocator))))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        if (FAILED(g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, tempAllocator, nullptr, IID_PPV_ARGS(&g_CmdList))))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        tempAllocator->Release();
        tempAllocator = nullptr;

        if (FAILED(g_CmdList->Close()))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = g_BufferCount;

        g_Device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_RTVHeap));

        // SRV heap (ImGui)
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        g_Device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_SrvHeap));

        // Create render targets + allocators
        auto rtvHandle = g_RTVHeap->GetCPUDescriptorHandleForHeapStart();
        UINT rtvSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (UINT i = 0; i < g_BufferCount; i++)
        {
            // Could be error prone!
            if (g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_FrameContext[i].CommandAllocator)) != S_OK)
                return OriginalPresent(pSwapChain, SyncInterval, Flags);

            if (pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_FrameContext[i].RenderTarget)) != S_OK)
                return OriginalPresent(pSwapChain, SyncInterval, Flags);

            g_Device->CreateRenderTargetView(
                g_FrameContext[i].RenderTarget,
                nullptr,
                rtvHandle
            );

            g_FrameContext[i].FenceValue = 0;
            g_FrameContext[i].RTV = rtvHandle;
            rtvHandle.ptr += rtvSize;
        }

        if (g_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (g_fenceEvent == nullptr)
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        ImGui_ImplWin32_Init(g_Hwnd);

        ImGui_ImplDX12_InitInfo initInfo = ImGui_ImplDX12_InitInfo();
        initInfo.CommandQueue = g_CommandQueue;
        initInfo.Device = g_Device;
        initInfo.DSVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.LegacySingleSrvCpuDescriptor = g_SrvHeap->GetCPUDescriptorHandleForHeapStart();
        initInfo.LegacySingleSrvGpuDescriptor = g_SrvHeap->GetGPUDescriptorHandleForHeapStart();
        initInfo.NumFramesInFlight = g_BufferCount;
        initInfo.SrvDescriptorHeap = g_SrvHeap;
        ImGui_ImplDX12_Init(&initInfo);

        if (!g_WndProcHooked)
        {
            OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HookWndProc)));
            g_WndProcHooked = true;
        }

        ImGui_ImplDX12_CreateDeviceObjects();

        g_Initialized = true;
    }

    //if (!g_MenuActive)
    //    return OriginalPresent(pSwapChain, SyncInterval, Flags);

    if (!g_CommandQueue)
        return OriginalPresent(pSwapChain, SyncInterval, Flags);

    if (!g_Swapchain3)
    {
        pSwapChain->QueryInterface(IID_PPV_ARGS(&g_Swapchain3));
    }

    UINT frameIndex = g_Swapchain3->GetCurrentBackBufferIndex();
    FrameContext& frame = g_FrameContext[frameIndex];

    if (g_fence->GetCompletedValue() < frame.FenceValue)
    {
        g_fence->SetEventOnCompletion(frame.FenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    if (!g_MenuActive)
        return OriginalPresent(pSwapChain, SyncInterval, Flags);

    // ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RenderMenu();

    // Reset
    if (frame.CommandAllocator->Reset() != S_OK)
        return OriginalPresent(pSwapChain, SyncInterval, Flags);

    // Transition: PRESENT → RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frame.RenderTarget;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    if (FAILED(g_CmdList->Reset(frame.CommandAllocator, nullptr)))
    {
        logger::println("g_CmdList->Reset failed");
    }
    g_CmdList->ResourceBarrier(1, &barrier);
    g_CmdList->OMSetRenderTargets(1, &frame.RTV, FALSE, nullptr);
    g_CmdList->SetDescriptorHeaps(1, &g_SrvHeap);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_CmdList);

    // Transition back
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_CmdList->ResourceBarrier(1, &barrier);
    g_CmdList->Close();

    g_CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_CmdList);

    g_CommandQueue->Signal(g_fence, ++g_fenceLastSignaledValue);
    frame.FenceValue = g_fenceLastSignaledValue;

    auto hr = OriginalPresent(pSwapChain, SyncInterval, Flags);
    if (hr != S_OK)
    {
        auto reason = g_Device->GetDeviceRemovedReason();
        logger::println("Issue with Present: %x", hr);
        logger::println("Reason: %x", reason);
    }
    return hr;
}


void WaitForPendingOperations()
{
    g_CommandQueue->Signal(g_fence, ++g_fenceLastSignaledValue);

    g_fence->SetEventOnCompletion(g_fenceLastSignaledValue, g_fenceEvent);
    ::WaitForSingleObject(g_fenceEvent, INFINITE);
}

static HRESULT STDMETHODCALLTYPE Hook_ResizeBuffer(IDXGISwapChain* p_This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    // Clear RenderTarget first
    if (g_Initialized)
    {
        WaitForPendingOperations();
        for (UINT i = 0; i < BufferCount; i++)
        {
            FrameContext& frame = g_FrameContext[i];
            if (frame.RenderTarget)
            {
                frame.RenderTarget->Release();
                frame.RenderTarget = nullptr;
            }
        }
    }

    // Continue with Original
    auto hr = OriginalResizeBuffer(p_This, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    // Afterwards CreateRenderTarget again! (ImGui win32 directx12 example)
    if (g_Initialized)
    {
        for (UINT i = 0; i < BufferCount; i++)
        {
            FrameContext& frame = g_FrameContext[i];
            ID3D12Resource* pBackBuffer = nullptr;
            p_This->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            g_Device->CreateRenderTargetView(pBackBuffer, nullptr, frame.RTV);
            frame.RenderTarget = pBackBuffer;
        }
    }

    return hr;
}

static void Hook_ExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
    // Receive the CommandQueue Pointer once
    if (!g_CommandQueue)
        g_CommandQueue = queue;

    OrginalExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
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

    IDXGIFactory4* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        DestroyWindow(hwnd);
        return false;
    }

    ID3D12Device* device = nullptr;
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device))))
    {
        factory->Release();
        DestroyWindow(hwnd);
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC qdesc = {};
    qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12CommandQueue* queue = nullptr;
    if (FAILED(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue))))
    {
        device->Release();
        factory->Release();
        DestroyWindow(hwnd);
        return false;
    }

    DXGI_SWAP_CHAIN_DESC scdesc = {};
    scdesc.BufferCount = 2;
    scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scdesc.OutputWindow = hwnd;
    scdesc.SampleDesc.Count = 1;
    scdesc.Windowed = TRUE;
    scdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    IDXGISwapChain* swapchain = nullptr;
    if (FAILED(factory->CreateSwapChain(queue, &scdesc, &swapchain)))
    {
        queue->Release();
        device->Release();
        factory->Release();
        DestroyWindow(hwnd);
        return false;
    }

    {
        void** vtable = *reinterpret_cast<void***>(swapchain);
        Present_Func = vtable[8];
        ResizeBuffers_Func = vtable[13];
    }

    {
        void** vtable = *reinterpret_cast<void***>(queue);
        ExecuteCommandLists_Func = vtable[10];
    }


    // Cleanup
    swapchain->Release();
    queue->Release();
    device->Release();
    factory->Release();
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
    HMODULE xinput1_4Handle = GetModuleHandleA("xinput1_4.dll");
    if (xinput1_4Handle == NULL)
    {
        logger::println("No xinput1_4.dll imported");
        return false;
    }

    XInputGetState_Func = GetProcAddress(xinput1_4Handle, "XInputGetState");
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

    //ExecuteCommandLists_t executeCommandLists = reinterpret_cast<ExecuteCommandLists_t>(ExecuteCommandLists_Func);
    if (!ExecuteCommandLists_Func)
    {
        logger::println("Getting ExecuteCommandLists vtable function failed!");
        return false;
    }

    if (MH_CreateHook(ExecuteCommandLists_Func, &Hook_ExecuteCommandLists, (LPVOID*)&OrginalExecuteCommandLists) != MH_OK)
    {
        logger::println("MH: Hook ExecuteCommandLists failed");
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

    //XInputGetState_t xInputGetState = reinterpret_cast<XInputGetState_t>(XInputGetState_Func);
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
    logger::println("Address of ExecuteCommandLists_Func: %p", ExecuteCommandLists_Func);
    logger::println("Address of GetDeviceState_Func: %p", GetDeviceState_Func);
    logger::println("Address of GetRawInputData_Func: %p", GetRawInputData_Func);

    return true;
}

void UninitializeImGui()
{
    if (g_Initialized)
    {
        WaitForPendingOperations();
        g_Initialized = false; // force re-init
        g_CommandQueue = nullptr;

        ImGui_ImplDX12_InvalidateDeviceObjects();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (g_Swapchain3)
        {
            g_Swapchain3->Release();
            g_Swapchain3 = nullptr;
        }

        for (UINT i = 0; i < g_BufferCount; i++)
        {
            if (g_FrameContext[i].RenderTarget)
            {
                g_FrameContext[i].RenderTarget->Release();
                g_FrameContext[i].RenderTarget = nullptr;
            }
            if (g_FrameContext[i].CommandAllocator) 
            {
                g_FrameContext[i].CommandAllocator->Release();
                g_FrameContext[i].CommandAllocator = nullptr;
            }
        }

        if (g_CmdList)
        {
            g_CmdList->Release();
            g_CmdList = nullptr;
        }

        if (g_RTVHeap)
        {
            g_RTVHeap->Release();
            g_RTVHeap = nullptr;
        }

        if (g_SrvHeap)
        {
            g_SrvHeap->Release();
            g_SrvHeap = nullptr;
        }

        if (g_fence) { g_fence->Release(); g_fence = nullptr; }
        if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }

        delete[] g_FrameContext;
        g_Device = nullptr;
        g_FrameContext = nullptr;
    }
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