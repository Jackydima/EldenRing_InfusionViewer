#pragma once

#include <windows.h>

#include "../tools/debug_print.h"
#include "../tools/memory.h"
#include "bases/spEffectParam.h"

namespace bases
{
	extern uintptr_t GameDataMan;
	extern uintptr_t WorldChrMan;
	extern uintptr_t SoloParamRepository;
	extern SpEffectParam SpEffectParamInst;
	bool initialize();

	using AddEffect_t = void(*__stdcall)(uintptr_t a_SelfPtr, int64_t a_EffectID, bool a_bDebug);
	extern AddEffect_t g_AddEffect;

	using RemoveEffect_t = int64_t(*__stdcall)(uintptr_t a_SpecialEffect, int32_t a_EffectID);
	extern RemoveEffect_t g_RemoveEffect;

	namespace characterEquipmentOffset
	{
		extern const std::vector<intptr_t> playerInstance;

		extern const std::vector<intptr_t> primRightWep;
		extern const std::vector<intptr_t> primLeftWep;

		extern const std::vector<intptr_t> secRightWep;
		extern const std::vector<intptr_t> secLeftWep;

		extern const std::vector<intptr_t> tertRightWep;
		extern const std::vector<intptr_t> tertLeftWep;

		extern const std::vector<intptr_t> currentLeftWep;
		extern const std::vector<intptr_t> currentRightWep;
		extern const std::vector<intptr_t> currentArmStyle;
	}
}
