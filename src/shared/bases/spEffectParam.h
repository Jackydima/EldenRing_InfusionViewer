#pragma once

#include <windows.h>
#include <map>

#include "Effect.h"
#include "../../tools/memory.h"

const int64_t EffectIdAddressJumpOffset = 0x18;

namespace bases
{
	// Maybe make it a signleton!
	class SpEffectParam
	{
	private:
		uintptr_t Base;
		WORD Count;
		uintptr_t EffectIdAddress;
		uintptr_t EffectOffsetAddress;

		// Mappes Effect ID to Offsets
		std::map<int32_t, int32_t> EffecIdtMap;

		// Stored original effect structs
		std::map<int32_t, Effect> OriginalEffects;

		EffectData* GetEffectById(int a_ID);

	public:
		SpEffectParam();
		~SpEffectParam();
		void init(uintptr_t SoloParamRepository);

		uintptr_t GetBase();
		WORD GetSize();
		uintptr_t GetEffectIdAddress();
		uintptr_t GetEffectOffsetAddress();
		
		EffectData* StartEffectModdingById(int a_ID);
		bool RestoreEffectById(int a_ID);
		void RestoreOriginalState();
	};
}