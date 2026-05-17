#pragma once

#include <Windows.h>
#include <MinHook.h>

#include "../../tools/debug_print.h"
#include "../../tools/memory.h"
#include "../params/effect/spEffectParam.h"

namespace bases
{
	extern uintptr_t GameDataMan;
	extern uintptr_t WorldChrMan;
	extern uintptr_t SoloParamRepository;
	extern SpEffectParam SpEffectParamInst;
	bool initialize();
	
	uintptr_t** getPlayerPtrByIndex(int a_Index);

	using AddEffect_t = void(*__stdcall)(uintptr_t* a_SelfPtr, int64_t a_EffectID, bool a_bDebug);
	extern AddEffect_t AddEffect;

	using RemoveEffect_t = int64_t(*__stdcall)(uintptr_t* a_SpecialEffect, int32_t a_EffectID);
	extern RemoveEffect_t RemoveEffect;

	using CallVfx_f = void(*__stdcall)(uintptr_t* a_pCSChrVfxModule, int64_t vfxId, uintptr_t* a_pSPEffectObject);
	extern CallVfx_f CallVfx;

	using PrepareVfxResource_f = void(*__stdcall)(VFXResource* a_pVfxResource, int64_t a_VfxId, EffectData* a_pEffect );
	extern PrepareVfxResource_f PrepareVfxResource;

	using GetEffectDataById_t = void(*__stdcall)(GameEffectData* a_pEffectDataStruct, int32_t effectId);
	extern GetEffectDataById_t GetEffectDataById;

	using HasEffectId_t = bool(*__stdcall)(uintptr_t* a_pSpecialEffectPtr, int32_t effectId);
	extern HasEffectId_t HasEffectId;

	namespace playerGameDataOffset
	{
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

	namespace charactersOffset
	{
		extern const std::vector<intptr_t> playerInstance;
		extern const std::vector<intptr_t> netPlayer1;
		extern const std::vector<intptr_t> netPlayer2;
		extern const std::vector<intptr_t> netPlayer3;
		extern const std::vector<intptr_t> netPlayer4;
		extern const std::vector<intptr_t> netPlayer5;
	}

	namespace playerOffsets
	{
		extern const std::vector<intptr_t> debugPhantomColor;
		extern const std::vector<intptr_t> moduleListPtr;
		extern const std::vector<intptr_t> moduleChrVfxModule;
		extern const std::vector<intptr_t> moduleChrSfxModule;
	}
}
