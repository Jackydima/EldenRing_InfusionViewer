#include "menu.h"

const int HOTKEYID = 1999;

// Globals
using Present_t = HRESULT(__stdcall*)(IDXGISwapChain* p_SwapChain, UINT SyncInterval, UINT Flags);
Present_t OriginalPresent = nullptr;
void* PresentPtr = nullptr;
void* ExecuteCommandListsPtr = nullptr;

using ExecuteCommandLists_t = void(__stdcall*)(ID3D12CommandQueue*,UINT, ID3D12CommandList* const);
ExecuteCommandLists_t OrginalExecuteCommandLists = nullptr;

FrameContext* g_FrameContext = nullptr;
UINT g_BufferCount = 0;

HWND g_Hwnd = nullptr;
ID3D12Device* g_Device = nullptr;
ID3D12DescriptorHeap* g_RTVHeap = nullptr;
ID3D12DescriptorHeap* g_SrvHeap = nullptr;
ID3D12CommandQueue* g_CommandQueue = nullptr;
ID3D12GraphicsCommandList* g_CmdList = nullptr;

bool g_Initialized = false;
bool* g_Running = nullptr;

WNDPROC OriginalWndProc = 0;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_HOTKEY:
        if (wParam != HOTKEYID)
            break;

        if (g_Running != nullptr)
            *g_Running = false;
        break;
    }

    return CallWindowProcW(OriginalWndProc, hWnd, msg, wParam, lParam);
}

// x64 this call has rcx for std call in the first parameter!
static HRESULT __stdcall Hook_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    IDXGISwapChain3* swapchain3 = nullptr;

    if (!g_Initialized)
    {
        if (FAILED(pSwapChain->QueryInterface(IID_PPV_ARGS(&swapchain3))))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        g_Hwnd = desc.OutputWindow;

        g_BufferCount = desc.BufferCount;

        // Device
        if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_Device))))
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        if (!g_CommandQueue)
            return OriginalPresent(pSwapChain, SyncInterval, Flags);

        // Allocate frame contexts
        g_FrameContext = new FrameContext[g_BufferCount];

        // Create a temporary allocator for command list creation
        ID3D12CommandAllocator* tempAllocator = nullptr;
        g_Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&tempAllocator)
        );

        // Create command list
        g_Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            tempAllocator,
            nullptr,
            IID_PPV_ARGS(&g_CmdList)
        );

        g_CmdList->Close();

        // RTV heap
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
            g_Device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&g_FrameContext[i].CommandAllocator)
            );

            pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_FrameContext[i].RenderTarget));

            g_Device->CreateRenderTargetView(
                g_FrameContext[i].RenderTarget,
                nullptr,
                rtvHandle
            );

            g_FrameContext[i].RTV = rtvHandle;
            rtvHandle.ptr += rtvSize;
        }

        // ImGui init
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(g_Hwnd);

        ImGui_ImplDX12_Init(
            g_Device,
            g_BufferCount,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            g_SrvHeap,
            g_SrvHeap->GetCPUDescriptorHandleForHeapStart(),
            g_SrvHeap->GetGPUDescriptorHandleForHeapStart()
        );

        OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HookWndProc)));
        if (FALSE == RegisterHotKey(g_Hwnd, HOTKEYID, MOD_ALT, VK_NUMPAD1)) {
            logger::println("HotKey not registered :/");
        }

        g_Initialized = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.Fonts->IsBuilt())
        io.Fonts->Build();

    if (!g_CommandQueue)
        return OriginalPresent(pSwapChain, SyncInterval, Flags);

    if (!swapchain3)
        pSwapChain->QueryInterface(IID_PPV_ARGS(&swapchain3));

    UINT frameIndex = swapchain3->GetCurrentBackBufferIndex();
    FrameContext& frame = g_FrameContext[frameIndex];

    // Reset
    frame.CommandAllocator->Reset();
    g_CmdList->Reset(frame.CommandAllocator, nullptr);

    // Transition: PRESENT → RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = frame.RenderTarget;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    g_CmdList->ResourceBarrier(1, &barrier);

    // Set render target
    g_CmdList->OMSetRenderTargets(1, &frame.RTV, FALSE, nullptr);

    // ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();

    // Bind heap
    ID3D12DescriptorHeap* heaps[] = { g_SrvHeap };
    g_CmdList->SetDescriptorHeaps(1, heaps);

    // Render ImGui
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_CmdList);

    // Transition back
    std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    g_CmdList->ResourceBarrier(1, &barrier);

    // Execute
    g_CmdList->Close();

    ID3D12CommandList* lists[] = { g_CmdList };
    g_CommandQueue->ExecuteCommandLists(1, lists);

    return OriginalPresent(pSwapChain, SyncInterval, Flags);
}

void Hook_ExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
    if (!g_CommandQueue)
        g_CommandQueue = queue;

    OrginalExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

static bool InitDXGIPointers()
{
    // Create dummy window
    HWND hwnd = CreateWindowA("STATIC", "dummy", WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr);

    IDXGIFactory4* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return false;

    ID3D12Device* device = nullptr;
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device))))
        return false;

    D3D12_COMMAND_QUEUE_DESC qdesc = {};
    qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12CommandQueue* queue = nullptr;
    if (FAILED(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue))))
        return false;

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
        return false;

    void** vtable = *reinterpret_cast<void***>(swapchain);
    PresentPtr = vtable[8];

    void** queueVtable = *reinterpret_cast<void***>(queue);
    ExecuteCommandListsPtr = queueVtable[10];


    // Cleanup
    swapchain->Release();
    queue->Release();
    device->Release();
    factory->Release();
    DestroyWindow(hwnd);

    return true;
}

bool InitMenu(bool* a_pRunningParam)
{
    g_Running = a_pRunningParam;

    if (!InitDXGIPointers())
        return false;

    Present_t present = reinterpret_cast<Present_t>(PresentPtr);
    if (!present)
    {
        logger::println("Getting Present vtable function failed!");
        return false;
    }

    ExecuteCommandLists_t executeCommandLists = reinterpret_cast<ExecuteCommandLists_t>(ExecuteCommandListsPtr);

    if (MH_Initialize() != MH_OK)
    {
        logger::println("MH: Init failed");
        return false;
    }

    if (MH_CreateHook(present, &Hook_Present, (LPVOID*)&OriginalPresent) != MH_OK)
    {
        logger::println("MH: Creating Hook Present failed");
        return false;
    }

    if (MH_CreateHook(executeCommandLists, &Hook_ExecuteCommandLists, (LPVOID*)&OrginalExecuteCommandLists) != MH_OK)
    {
        logger::println("MH: Creating Hook OrginalExecuteCommandLists failed");
        return false;
    }

    if (MH_EnableHook(present) != MH_OK)
    {
        logger::println("MH: Enable Hooking failed");
        return false;
    }

    if (MH_EnableHook(executeCommandLists) != MH_OK)
    {
        logger::println("MH: Enable executeCommandLists Hooking failed");
        return false;
    }

    logger::println("Found Present Function: %p", PresentPtr);
    logger::println("Found ExecuteCommandLists Function: %p", ExecuteCommandListsPtr);
    return true;
}

bool CleanUpMenu()
{
    SetWindowLongPtrW(g_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OriginalWndProc));

    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
        return false;

    return MH_Uninitialize() == MH_OK;
}