
#include "main.h"

EffectData* selfVisual;

enum WeaponInfusion
{
    WeaponInfusion_Fire = 4,
    WeaponInfusion_Fire2 = 5,
    WeaponInfusion_Lightning = 6,
    WeaponInfusion_Sacred = 7,
    WeaponInfusion_Magic = 8,
    WeaponInfusion_Cold = 9,
    WeaponInfusion_Poison = 10,
    WeaponInfusion_Blood = 11,
    WeaponInfusion_Ocult = 12,
};

static WeaponInfusion getInfusionValue(int32_t a_iWeaponID)
{
    return static_cast<WeaponInfusion>((a_iWeaponID / 100) % 100);
}

static void ProcessPlayerInfusion(uintptr_t currentPlayer)
{
    if (!memory::isReadable(currentPlayer))
        return;

    int32_t* currentRightWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::currentRightWep);
    int32_t* currentLeftWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::currentLeftWep);
    int8_t* currentArmStyle = memory::readPointerSafe<int8_t*>(currentPlayer, bases::playerEquipmentOffset::currentArmStyle);

    if (!currentRightWep || !currentLeftWep || !currentArmStyle)
        return;

    int32_t* primRightWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::primRightWep);
    int32_t* primLeftWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::primLeftWep);
    int32_t* secRightWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::secRightWep);
    int32_t* secLeftWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::secLeftWep);
    int32_t* tertRightWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::tertRightWep);
    int32_t* tertLeftWep = memory::readPointerSafe<int32_t*>(currentPlayer, bases::playerEquipmentOffset::tertLeftWep);

    if (!primRightWep || !primLeftWep || !secRightWep || !secLeftWep || !tertRightWep || !tertLeftWep)
        return;

    int32_t rightItemID = 0;
    int32_t leftItemID = 0;

    switch (*currentRightWep)
    {
    case 0:
        rightItemID = *primRightWep;
        break;
    case 1:
        rightItemID = *secRightWep;
        break;
    case 2:
        rightItemID = *tertRightWep;
        break;
    default:
        return;
    }

    switch (*currentLeftWep)
    {
    case 0:
        leftItemID = *primLeftWep;
        break;
    case 1:
        leftItemID = *secLeftWep;
        break;
    case 2:
        leftItemID = *tertLeftWep;
        break;
    default:
        return;
    }

    int32_t rightVfxID = -1;
    switch (getInfusionValue(rightItemID))
    {
    case WeaponInfusion_Fire:
    case WeaponInfusion_Fire2:
        rightVfxID = g_FireEffectR;
        break;
    case WeaponInfusion_Lightning:
        rightVfxID = g_LightningEffectR;
        break;
    case WeaponInfusion_Sacred:
        rightVfxID = g_SacradEffectR;
        break;
    case WeaponInfusion_Magic:
        rightVfxID = g_MagicEffectR;
        break;
    case WeaponInfusion_Cold:
        rightVfxID = g_ColdEffectR;
        break;
    case WeaponInfusion_Poison:
        rightVfxID = g_PoisonEffectR;
        break;
    case WeaponInfusion_Blood:
        rightVfxID = g_BloodEffectR;
        break;
    case WeaponInfusion_Ocult:
        rightVfxID = -1;
        break;

    default: break;
    }

    int32_t leftVfxID = -1;
    switch (getInfusionValue(leftItemID))
    {
    case WeaponInfusion_Fire:
    case WeaponInfusion_Fire2:
        leftVfxID = g_FireEffectL;
        break;
    case WeaponInfusion_Lightning:
        leftVfxID = g_LightningEffectL;
        break;
    case WeaponInfusion_Sacred:
        leftVfxID = g_SacradEffectL;
        break;
    case WeaponInfusion_Magic:
        leftVfxID = g_MagicEffectL;
        break;
    case WeaponInfusion_Cold:
        leftVfxID = g_ColdEffectL;
        break;
    case WeaponInfusion_Poison:
        leftVfxID = g_PoisonEffectL;
        break;
    case WeaponInfusion_Blood:
        leftVfxID = g_BloodEffectL;
        break;
    case WeaponInfusion_Ocult:
        leftVfxID = -1;
        break;

    default: break;
    }

    // Fix for effect glitch when 2H holding left weapon!
    if (*currentArmStyle == 2)
    {
        rightVfxID = -1;
    }

    selfVisual->vfxId = rightVfxID;
    selfVisual->vfxId1 = leftVfxID;

    bases::AddEffect(*reinterpret_cast<uintptr_t**>(currentPlayer), 17, true);
}

static void MainLoop()
{
    uintptr_t** currentPlayer;
    for (int i = 0; i < 6; i++)
    {
        currentPlayer = bases::getPlayerPtrByIndex(i);

        if (!currentPlayer)
            continue;

        ProcessPlayerInfusion(reinterpret_cast<uintptr_t>(currentPlayer));
    }
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
    while (!bases::initialize())
    {
        logger::println("Initilializing bases went wrong :/");
        Sleep(1000);
    }

    // Init Effect Modding
    selfVisual = bases::SpEffectParamInst.StartEffectModdingById(17);
    if (selfVisual == nullptr)
    {
        logger::println("Nullpointer EffectData");
        FreeLibraryAndExitThread((HMODULE)lpParam, 0);
        return 0;
    }

    selfVisual->effectEndurance = -1.0f;
    selfVisual->spCategory = 0;
    selfVisual->effectTargetSelf = 1; // TargetSelf Active!
    
    while (true)
    {
        if (GetAsyncKeyState(VK_F10) < 0)
            break;

        MainLoop();
        Sleep(50);
    }

    {
        uintptr_t** currentPlayer;
        for (int i = 0; i < 6; i++)
        {
            currentPlayer = bases::getPlayerPtrByIndex(i);

            if (!currentPlayer || !*currentPlayer)
                continue;

            bases::RemoveEffect(*memory::readOffSet<uintptr_t**>(*reinterpret_cast<uintptr_t*>(currentPlayer), 0x178), 17);
        }
    }
    
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

