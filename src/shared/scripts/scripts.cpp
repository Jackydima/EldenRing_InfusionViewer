

#include "scripts.h"

int g_EffectList[PLAYER_AMOUNT] = { 3001, 3002, 3003, 3004, 3005, 3006 };
const int customExtraEffectId = 3101;

void ProcessCustomVisual()
{
    static bool active = false;

    if (!active && !config::ExtraVisualActive)
        return;

    uintptr_t selfPlayerAddr = reinterpret_cast<uintptr_t>(bases::getPlayerPtrByIndex(0));
    if (!selfPlayerAddr || !*reinterpret_cast<uintptr_t*>(selfPlayerAddr))
        return;

    uintptr_t* specialEffectPtr = *memory::readOffSet<uintptr_t**>(*reinterpret_cast<uintptr_t*>(selfPlayerAddr), 0x178);

    if (!config::ExtraVisualActive)
    {
        if (!specialEffectPtr || !*specialEffectPtr)
            return;
        if (bases::HasEffectId(specialEffectPtr, customExtraEffectId))
        {
            bases::RemoveEffect(specialEffectPtr, customExtraEffectId);
        }

        active = false;
        return;
    }

    EffectData* effectData = bases::SpEffectParamInst.GetCustomEffectById(customExtraEffectId);
    if (!effectData)
        return;

    effectData->vfxId = config::ExtraVFX;
    effectData->vfxId1 = config::ExtraVFX1;
    effectData->vfxId2 = config::ExtraVFX2;
    effectData->vfxId3 = config::ExtraVFX3;
    effectData->vfxId4 = config::ExtraVFX4;
    effectData->vfxId5 = config::ExtraVFX5;
    effectData->vfxId6 = config::ExtraVFX6;
    effectData->vfxId7 = config::ExtraVFX7;

    bases::AddEffect(*reinterpret_cast<uintptr_t**>(selfPlayerAddr), customExtraEffectId, true);
    active = true;
}

WeaponInfusion getInfusionValue(int32_t a_iWeaponID)
{
    return static_cast<WeaponInfusion>((a_iWeaponID / 100) % 100);
}

void ProcessPlayerInfusion(int a_PlayerIndex, int a_iEffectID)
{
    static bool active[PLAYER_AMOUNT] = { false };

    // Return early if infusionviewer is not active
    if (!active[a_PlayerIndex] && !config::InfusionViewerActive)
        return;

    uintptr_t currentPlayerAdr = reinterpret_cast<uintptr_t>(bases::getPlayerPtrByIndex(a_PlayerIndex));
    if (!currentPlayerAdr || !*reinterpret_cast<uintptr_t*>(currentPlayerAdr))
        return;

    if (!config::InfusionViewerActive)
    {
        uintptr_t* specialEffectPtr = *memory::readOffSet<uintptr_t**>(*reinterpret_cast<uintptr_t*>(currentPlayerAdr), 0x178);
        if (!specialEffectPtr || !*specialEffectPtr)
            return;
        if (bases::HasEffectId(specialEffectPtr, a_iEffectID))
        {
            bases::RemoveEffect(specialEffectPtr, a_iEffectID);
        }

        active[a_PlayerIndex] = false;
        return;
    }

    int32_t* currentRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentRightWep);
    int32_t* currentLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentLeftWep);
    int8_t* currentArmStyle = memory::readPointer<int8_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentArmStyle);

    if (!currentRightWep || !currentLeftWep || !currentArmStyle)
        return;

    int32_t* primRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::primRightWep);
    int32_t* primLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::primLeftWep);
    int32_t* secRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::secRightWep);
    int32_t* secLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::secLeftWep);
    int32_t* tertRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::tertRightWep);
    int32_t* tertLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::tertLeftWep);

    if (!primRightWep || !primLeftWep || !secRightWep || !secLeftWep || !tertRightWep || !tertLeftWep)
        return;

    EffectData* effectData = bases::SpEffectParamInst.GetCustomEffectById(a_iEffectID);
    if (!effectData)
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
        rightVfxID = config::VFX::g_FireEffectR;
        break;
    case WeaponInfusion_Lightning:
        rightVfxID = config::VFX::g_LightningEffectR;
        break;
    case WeaponInfusion_Sacred:
        rightVfxID = config::VFX::g_SacradEffectR;
        break;
    case WeaponInfusion_Magic:
        rightVfxID = config::VFX::g_MagicEffectR;
        break;
    case WeaponInfusion_Cold:
        rightVfxID = config::VFX::g_ColdEffectR;
        break;
    case WeaponInfusion_Poison:
        rightVfxID = config::VFX::g_PoisonEffectR;
        break;
    case WeaponInfusion_Blood:
        rightVfxID = config::VFX::g_BloodEffectR;
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
        leftVfxID = config::VFX::g_FireEffectL;
        break;
    case WeaponInfusion_Lightning:
        leftVfxID = config::VFX::g_LightningEffectL;
        break;
    case WeaponInfusion_Sacred:
        leftVfxID = config::VFX::g_SacradEffectL;
        break;
    case WeaponInfusion_Magic:
        leftVfxID = config::VFX::g_MagicEffectL;
        break;
    case WeaponInfusion_Cold:
        leftVfxID = config::VFX::g_ColdEffectL;
        break;
    case WeaponInfusion_Poison:
        leftVfxID = config::VFX::g_PoisonEffectL;
        break;
    case WeaponInfusion_Blood:
        leftVfxID = config::VFX::g_BloodEffectL;
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

    // Fix for Swapping Weapons with same infusion
    static int32_t LastRightWeaponArray[PLAYER_AMOUNT] = { -1, -1, -1, -1, -1, -1 };
    if (LastRightWeaponArray[a_PlayerIndex] != rightItemID)
    {
        rightVfxID = -1;
    }
    LastRightWeaponArray[a_PlayerIndex] = rightItemID;

    static int32_t LastLeftWeaponArray[PLAYER_AMOUNT] = { -1, -1, -1, -1, -1, -1 };
    if (LastLeftWeaponArray[a_PlayerIndex] != leftItemID)
    {
        leftVfxID = -1;
    }
    LastLeftWeaponArray[a_PlayerIndex] = leftItemID;


    effectData->vfxId = rightVfxID;
    effectData->vfxId1 = leftVfxID;

    bases::AddEffect(*reinterpret_cast<uintptr_t**>(currentPlayerAdr), a_iEffectID, true);

    active[a_PlayerIndex] = true;
}

void RemoveEffectForPlayers()
{
    uintptr_t** currentPlayer;
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        currentPlayer = bases::getPlayerPtrByIndex(i);

        if (!currentPlayer || !*currentPlayer)
            continue;

        if (i == 0)
        {
            // SelfPlayer
            bases::RemoveEffect(*memory::readOffSet<uintptr_t**>(*reinterpret_cast<uintptr_t*>(currentPlayer), 0x178), customExtraEffectId);
        }
        bases::RemoveEffect(*memory::readOffSet<uintptr_t**>(*reinterpret_cast<uintptr_t*>(currentPlayer), 0x178), g_EffectList[i]);
    }
}

bool InitInfusionEffects()
{
    // Init Effect Modding
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        //EffectData* effectData = bases::SpEffectParamInst.StartEffectModdingById(g_EffectList[i]);
        if (!InitVisualCustomEffect(g_EffectList[i]))
            return false;
    }

    // Extra Effect
    if (!InitVisualCustomEffect(customExtraEffectId))
        return false;
    return true;
}

bool InitVisualCustomEffect(int a_iEffectID)
{
    //EffectData* effectData = bases::SpEffectParamInst.StartEffectModdingById(g_EffectList[i]);
    EffectData* effectData = bases::SpEffectParamInst.CreateCustomEffect(a_iEffectID);
    if (effectData == nullptr)
    {
        logger::println("Nullpointer EffectData");
        return false;
    }

    // Good Base Effect with default values!
    const EffectData* baseEffectData = bases::SpEffectParamInst.GetEffectById(46);
    *effectData = *baseEffectData; // Copy base values

    // Still for safety
    effectData->effectEndurance = -1.0f;
    effectData->spCategory = 0;
    effectData->stateInfo = 0;
    effectData->vowType0 = 1;
    effectData->vowType1 = 1;
    effectData->vowType2 = 1;
    effectData->vowType3 = 1;
    effectData->vowType4 = 1;
    effectData->vowType5 = 1;
    effectData->vowType6 = 1;
    effectData->vowType7 = 1;
    effectData->vowType8 = 1;
    effectData->vowType9 = 1;
    effectData->vowType10 = 1;
    effectData->vowType11 = 1;
    effectData->vowType12 = 1;
    effectData->vowType13 = 1;
    effectData->vowType14 = 1;
    effectData->vowType15 = 1;

    effectData->effectTargetSelfTarget = 1;
    effectData->effectTargetSelf = 1;
    effectData->effectTargetPlayer = 1;
    effectData->effectTargetOpposeTarget = 1;
    effectData->effectTargetEnemy = 1;
    effectData->effectTargetFriend = 1;

    return true;
}

void DeactivatePhantomColor()
{
    uintptr_t** currentPlayer;
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        currentPlayer = bases::getPlayerPtrByIndex(i);
        if (!currentPlayer || !*currentPlayer)
            return;

        int32_t* debugPhantomColor = memory::readPointer<int32_t*>(reinterpret_cast<uintptr_t>(currentPlayer), bases::playerOffsets::debugPhantomColor);
        if (!debugPhantomColor)
            return;

        *debugPhantomColor = -1;
    }
}

void SetDebugPhantomColor(int a_PlayerIndex)
{
    static bool active[PLAYER_AMOUNT] = { false };
    // Early return if is was deactivated and phantomcolor was not active at all.
    if (!active[a_PlayerIndex] && !config::PhantomColorActive)
        return;

    uintptr_t** pCurrentPlayer= bases::getPlayerPtrByIndex(a_PlayerIndex);
    if (!pCurrentPlayer || !*pCurrentPlayer)
        return;

    int32_t* debugPhantomColor = memory::readPointer<int32_t*>(reinterpret_cast<uintptr_t>(pCurrentPlayer), bases::playerOffsets::debugPhantomColor);
    if (!debugPhantomColor)
        return;

    // Deactivate the active custom phantomcoloring
    if (!config::PhantomColorActive)
    {
        *debugPhantomColor = -1;
        active[a_PlayerIndex] = false;
        return;
    }

    switch (a_PlayerIndex)
    {
    case 0: *debugPhantomColor = config::PhantomSelfId; break;
    case 1: *debugPhantomColor = config::NetPlayer1Id; break;
    case 2: *debugPhantomColor = config::NetPlayer2Id; break;
    case 3: *debugPhantomColor = config::NetPlayer3Id; break;
    case 4: *debugPhantomColor = config::NetPlayer4Id; break;
    case 5: *debugPhantomColor = config::NetPlayer5Id; break;
    default:
        break;
    }

    active[a_PlayerIndex] = true;
}
