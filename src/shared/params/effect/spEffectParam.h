#pragma once

#include <windows.h>
#include <map>

#include "Effect.h"
#include "../../../tools/memory.h"
#include "../params.h"

const int64_t EffectIdAddressJumpOffset = 0x18;

namespace bases
{
	// Maybe make it a signleton!
	class SpEffectParam : ParamBase
	{
	private:
		// Mappes Effect ID to Offsets
		std::map<int32_t, int32_t> EffecIdMap;

		// Stored original effect structs
		std::map<int32_t, Effect> OriginalEffects;

		// Custom Created Effects
		std::map<int32_t, EffectData> CustomEffectsMap;

		EffectData* GetEffectById(int a_ID);

	public:
		SpEffectParam();
		~SpEffectParam();
		
		bool init(uintptr_t SoloParamRepository) override;
		void RestoreOriginalState() override;
		EffectData* StartEffectModdingById(int a_ID);
		EffectData* CreateCustomEffect(int32_t a_EffectId);
		EffectData* GetCustomEffectById(int32_t a_EffectId);
		//EffectData* CreatedCustomEffectByEffect(int32_t a_EffectId, Effect& a_BaseEffect); // TODO


		bool RestoreEffectById(int a_ID);
	};
}