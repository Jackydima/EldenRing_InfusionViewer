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

WeaponInfusion getInfusionValue(int32_t a_iWeaponID);
void ProcessPlayerInfusion(int a_CurrentPlayerIndex, int a_iEffectID);
void RemoveEffectForPlayers();
bool InitInfusionEffects();

void DeactivatePhantomColor();
void SetDebugPhantomColor(int a_index);
