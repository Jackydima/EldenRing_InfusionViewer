
#include "main.h"

EffectData* selfVisual;
uintptr_t* PlayerPtr;

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



static void MainLoop()
{
    PlayerPtr = tools::memory::readPointer<uintptr_t*>(bases::WorldChrMan, bases::characterEquipmentOffset::playerInstance);

    if (!PlayerPtr)
        return;

    int32_t* currentRightWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::currentRightWep);
    int32_t* currentLeftWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::currentLeftWep);
    int8_t* currentArmStyle = tools::memory::readPointer<int8_t*>(bases::GameDataMan, bases::characterEquipmentOffset::currentArmStyle);

    if (!currentRightWep || !currentLeftWep || !currentArmStyle)
        return;

    int32_t* primRightWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::primRightWep);
    int32_t* primLeftWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::primLeftWep);
    int32_t* secRightWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::secRightWep);
    int32_t* secLeftWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::secLeftWep);
    int32_t* tertRightWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::tertRightWep);
    int32_t* tertLeftWep = tools::memory::readPointer<int32_t*>(bases::GameDataMan, bases::characterEquipmentOffset::tertLeftWep);

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

    // Fix for effect glicht!
    if (*currentArmStyle == 2)
    {
        rightVfxID = -1;
    }

    selfVisual->vfxId = rightVfxID;
    selfVisual->vfxId1 = leftVfxID;

    bases::g_AddEffect(reinterpret_cast<uintptr_t>(PlayerPtr), 17, true);
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

    logger::println("Started Main function!");

    if (!bases::initialize())
    {
        logger::println("Initilializing bases went wrong :/");
        return 0;
    }

    selfVisual = bases::SpEffectParamInst.StartEffectModdingById(17);
    if (selfVisual == nullptr)
    {
        logger::println("Nullpointer EffectData");
        return 0;
    }

    selfVisual->effectEndurance = -1.0f;
    selfVisual->vfxId = 2;
    selfVisual->spCategory = 0;
    selfVisual->effectTargetSelf = 1; // TargetSelf Active!
    
    while (true)
    {
        if (GetAsyncKeyState(VK_F10) < 0)
            break;

        MainLoop();
        Sleep(50);
    }

    if (PlayerPtr)
    {
        bases::g_RemoveEffect(*tools::memory::readOffSet<uintptr_t*>((uintptr_t)PlayerPtr, 0x178), 17);
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
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
#ifdef DEBUG
        FreeConsole();
#endif // DEBUG
        break;
    }
    return TRUE;
}

