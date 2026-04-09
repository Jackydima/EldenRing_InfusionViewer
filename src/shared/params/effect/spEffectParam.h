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

		EffectData* GetEffectById(int a_ID);

	public:
		SpEffectParam();
		~SpEffectParam();
		
		bool init(uintptr_t SoloParamRepository) override;
		void RestoreOriginalState() override;
		EffectData* StartEffectModdingById(int a_ID);
		bool RestoreEffectById(int a_ID);
	};
}