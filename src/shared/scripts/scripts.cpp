

#include "scripts.h"

int g_EffectList[PLAYER_AMOUNT] = { 14001, 14002, 14003, 14004, 14005, 14006 };

WeaponInfusion getInfusionValue(int32_t a_iWeaponID)
{
    return static_cast<WeaponInfusion>((a_iWeaponID / 100) % 100);
}

void ProcessPlayerInfusion(int a_PlayerIndex, int a_iEffectID)
{
    uintptr_t currentPlayerAdr = reinterpret_cast<uintptr_t>(bases::getPlayerPtrByIndex(a_PlayerIndex));
    if (!currentPlayerAdr || !*reinterpret_cast<uintptr_t*>(currentPlayerAdr))
        return;

    int32_t* currentRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentRightWep);
    int32_t* currentLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentLeftWep);
    int8_t* currentArmStyle = memory::readPointer<int8_t*>(currentPlayerAdr, bases::playerGameDataOffset::currentArmStyle);

    if (!currentRightWep || !currentLeftWep || !currentArmStyle)
    {
        logger::println("Current Status not found");
        return;
    }

    int32_t* primRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::primRightWep);
    int32_t* primLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::primLeftWep);
    int32_t* secRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::secRightWep);
    int32_t* secLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::secLeftWep);
    int32_t* tertRightWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::tertRightWep);
    int32_t* tertLeftWep = memory::readPointer<int32_t*>(currentPlayerAdr, bases::playerGameDataOffset::tertLeftWep);

    if (!primRightWep || !primLeftWep || !secRightWep || !secLeftWep || !tertRightWep || !tertLeftWep)
    {
        logger::println("Weapon Adresses not found");
        return;
    }

    EffectData* effectData = bases::SpEffectParamInst.GetCustomEffectById(a_iEffectID);
    if (!effectData)
    {
        logger::println("EffectData not found");
        return;
    }

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
    case WeaponInfusion_Chaos:
        rightVfxID = g_FireR;
        break;
    case WeaponInfusion_Lightning:
        rightVfxID = g_LightningR;
        break;
    case WeaponInfusion_Blessed:
        rightVfxID = g_BlessedR;
        break;
    case WeaponInfusion_Simple:
        rightVfxID = g_SimpleEffectR;
        break;
    case WeaponInfusion_Crystal:
        rightVfxID = g_CrystalR;
        break;
    case WeaponInfusion_Dark:
    case WeaponInfusion_Deep:
        rightVfxID = g_DarkR;
        break;
    case WeaponInfusion_Blood:
        rightVfxID = g_BloodEffectR;
        break;
    case WeaponInfusion_Poison:
        rightVfxID = g_PoisonEffectR;
        break;

    default: break;
    }

    int32_t leftVfxID = -1;
    switch (getInfusionValue(leftItemID))
    {
    case WeaponInfusion_Fire:
    case WeaponInfusion_Chaos:
        leftVfxID = g_FireL;
        break;
    case WeaponInfusion_Lightning:
        leftVfxID = g_LightningL;
        break;
    case WeaponInfusion_Blessed:
        leftVfxID = g_BlessedL;
        break;
    case WeaponInfusion_Simple:
        leftVfxID = g_SimpleEffectL;
        break;
    case WeaponInfusion_Crystal:
        leftVfxID = g_CrystalL;
        break;
    case WeaponInfusion_Dark:
    case WeaponInfusion_Deep:
        leftVfxID = g_DarkL;
        break;
    case WeaponInfusion_Blood:
        leftVfxID = g_BloodEffectL;
        break;
    case WeaponInfusion_Poison:
        leftVfxID = g_PoisonEffectL;
        break;

    default: break;
    }

    // Used for resetting Old Effect during same infusions
    static int32_t lastRightItemID = -1;
    static int32_t lastLeftItemID = -1;
    if (rightItemID != lastRightItemID)
    {
        rightVfxID = -1;
        lastRightItemID = rightItemID;
    }

    if (leftItemID != lastLeftItemID)
    {
        leftVfxID = -1;
        lastLeftItemID = leftItemID;
    }

    // Fix for effect glitch when 2H holding left weapon!
    if (*currentArmStyle == 2)
    {
        rightVfxID = -1;
    }

    effectData->vfxId = rightVfxID;
    effectData->vfxId1 = leftVfxID;

    bases::AddEffect(*reinterpret_cast<uintptr_t**>(currentPlayerAdr), a_iEffectID, *reinterpret_cast<uintptr_t**>(currentPlayerAdr));
}

void RemoveEffectForPlayers()
{
    uintptr_t** currentPlayer;
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        currentPlayer = bases::getPlayerPtrByIndex(i);

        if (!currentPlayer || !*currentPlayer)
            continue;

        bases::RemoveEffect(*memory::readPointer<uintptr_t**>(reinterpret_cast<uintptr_t>(currentPlayer), { 0x18, 0x18 }), g_EffectList[i]);
    }
}

bool InitInfusionEffects()
{
    // Init Effect Modding
    EffectData* reference = bases::SpEffectParamInst.GetEffectById(91);
    for (int i = 0; i < PLAYER_AMOUNT; i++)
    {
        //EffectData* effectData = bases::SpEffectParamInst.StartEffectModdingById(g_EffectList[i]);
        EffectData* effectData = bases::SpEffectParamInst.CreateCustomEffect(g_EffectList[i]);
        if (effectData == nullptr)
        {
            logger::println("Nullpointer EffectData");
            return false;
        }
        *effectData = *reference; // Copy Base Effect values first into our newly created effect

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

        if (i == 0) // SelfPlayer
        {
            effectData->effectTargetSelf = 1;
            effectData->effectTargetSelfTarget = 1;
            effectData->effectTargetPlayer = 1;
            effectData->effectTargetEnemy = 0;
            effectData->effectTargetFriend = 0;
            effectData->effectTargetEnemy = 0;

        }
        else
        {
            effectData->effectTargetSelfTarget = 0;
            effectData->effectTargetSelf = 0;
            effectData->effectTargetPlayer = 1;
            effectData->effectTargetOpposeTarget = 1;
            effectData->effectTargetEnemy = 1;
            effectData->effectTargetFriend = 1;
            effectData->effectTargetEnemy = 1;
        }
    }
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
    uintptr_t** pCurrentPlayer= bases::getPlayerPtrByIndex(a_PlayerIndex);
    if (!pCurrentPlayer || !*pCurrentPlayer)
        return;

    int32_t* debugPhantomColor = memory::readPointer<int32_t*>(reinterpret_cast<uintptr_t>(pCurrentPlayer), bases::playerOffsets::debugPhantomColor);
    if (!debugPhantomColor)
        return;

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
}
