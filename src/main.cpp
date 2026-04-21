
#include "main.h"

bool g_pRunning = true;

static void MainLoop()
{
    uintptr_t** currentPlayer;
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        currentPlayer = bases::getPlayerPtrByIndex(i);

        if (!currentPlayer || !memory::isReadable(reinterpret_cast<uintptr_t>(currentPlayer)))
            continue;

        if (config::InfusionViewerActive)
            ProcessPlayerInfusion(reinterpret_cast<uintptr_t>(currentPlayer), g_EffectList[i]);

        if (config::PhantomColorActive)
            SetDebugPhantomColor(i);
    }

    /*uintptr_t* selfPlayer = *bases::getPlayerPtrByIndex(0);
    uintptr_t* vfxModule = *memory::readPointerSafe<uintptr_t**>(reinterpret_cast<uintptr_t>(&selfPlayer), bases::playerOffsets::moduleChrVfxModule);
    uintptr_t* sfxModule = *memory::readPointerSafe<uintptr_t**>(reinterpret_cast<uintptr_t>(&selfPlayer), bases::playerOffsets::moduleChrSfxModule);
    EffectData* effectData = bases::SpEffectParamInst.StartEffectModdingById(3160);*/
}

static DWORD WINAPI MainThread(LPVOID lpParam)
{
#ifdef DEBUG
    AllocConsole();

    SetStdHandle(STD_INPUT_HANDLE, GetStdHandle(STD_INPUT_HANDLE));
    SetStdHandle(STD_OUTPUT_HANDLE, GetStdHandle(STD_OUTPUT_HANDLE));

    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
#endif // DEBUG

    if (MH_Initialize() != MH_OK)
    {
        logger::println("MH Failed init");
        FreeLibraryAndExitThread((HMODULE)lpParam, 0);
        return 0;
    }

    int tries = 10;
    while (!bases::initialize())
    {
        if (tries <= 0)
        {
            FreeLibraryAndExitThread((HMODULE)lpParam, 0);
            return 0;
        }
        logger::println("Initilializing bases went wrong :/");
        tries--;
        Sleep(1000);
    }

    DWORD delayTime = 2000;
    if (tries == 10)
        delayTime = 0;

    if (!InitMenu((HMODULE)lpParam, &g_pRunning, delayTime))
    {
        logger::println("Could not initilialize Hooking Rendering");
        FreeLibraryAndExitThread((HMODULE)lpParam, 0);
        return 0;
    }

    if (!InitInfusionEffects())
    {
        FreeLibraryAndExitThread((HMODULE)lpParam, 0);
        return 0;
    }
    
    while (g_pRunning)
    {
        MainLoop();
        Sleep(config::cycleSpeed);
    }

    RemoveEffectForPlayers();

    MH_DisableHook(MH_ALL_HOOKS); // Ignore errors here for now
    MH_Uninitialize();

    Sleep(100);
    CleanUpMenu();


    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr,0,MainThread,hModule,0,nullptr);
        break;
    case DLL_PROCESS_DETACH:
#ifdef DEBUG
        FreeConsole();
#endif // DEBUG
        break;
    }
    return TRUE;
}

