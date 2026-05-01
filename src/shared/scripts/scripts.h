#pragma once

#include "../../tools/memory.h"
#include "../../tools/debug_print.h"
#include "../../shared/bases/bases.h"
#include "../../menu/menu.h"
#include "../../shared/config.h"


const int PLAYER_AMOUNT = 6;
extern int g_EffectList[PLAYER_AMOUNT];

enum WeaponInfusion
{
    WeaponInfusion_Simple = 4,
    WeaponInfusion_Crystal = 5,
    WeaponInfusion_Fire = 6,
    WeaponInfusion_Chaos = 7,
    WeaponInfusion_Lightning = 8,
    WeaponInfusion_Deep = 9,
    WeaponInfusion_Dark = 10,
    WeaponInfusion_Poison = 11,
    WeaponInfusion_Blood = 12,
    WeaponInfusion_Blessed = 14,
};

WeaponInfusion getInfusionValue(int32_t a_iWeaponID);
void ProcessPlayerInfusion(int a_CurrentPlayerIndex, int a_iEffectID);
void RemoveEffectForPlayers();
bool InitInfusionEffects();

void DeactivatePhantomColor();
void SetDebugPhantomColor(int a_index);
