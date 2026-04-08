#include "spEffectParam.h"

namespace bases
{
    SpEffectParam::SpEffectParam() : Base(0), Count(0), EffectIdAddress(0), EffectOffsetAddress(0) {}

    void SpEffectParam::init(uintptr_t SoloParamRepository)
    {
        this->Base = *tools::memory::readPointer<uintptr_t*>(SoloParamRepository, { 0x4C0, 0x80, 0x80 });
        logger::println("SpEffectParam: %p", reinterpret_cast<LPVOID>(this->Base));
        this->Count = *tools::memory::readOffSet<WORD*>(this->Base, 0x0A);
        this->EffectIdAddress = this->Base + 0x40;
        this->EffectOffsetAddress = this->Base + 0x48;

        int32_t currentID;
        int32_t currentOffset;

        for (WORD i = 0; i < this->Count; i++)
        {
            currentID = *tools::memory::readOffSet<int32_t*>(this->EffectIdAddress, i * EffectIdAddressJumpOffset);
            currentOffset = *tools::memory::readOffSet<int32_t*>(this->EffectOffsetAddress, i * EffectIdAddressJumpOffset);
            this->EffecIdtMap.insert({currentID, currentOffset});
        }
    }

    EffectData* SpEffectParam::GetEffectById(int a_ID)
    {
        auto effectOffset = this->EffecIdtMap.find(a_ID);
        if (effectOffset == this->EffecIdtMap.end())
        {
            logger::println("Id not found in GetEffectById");
            return nullptr;
        }

        return reinterpret_cast<EffectData*>(this->Base + effectOffset->second);
    }

    // We want to modify when getting an effect, so create a backup!
    EffectData* SpEffectParam::StartEffectModdingById(int a_ID)
    {
        EffectData* effectData = GetEffectById(a_ID);
        if (effectData == nullptr)
        {
            return effectData;
        }

        if (this->OriginalEffects.find(a_ID) != this->OriginalEffects.end())
        {
            logger::println("Effect with ID (%d) already modified", a_ID);
            return effectData;
        }

        Effect effect = Effect();

        effect.data = *effectData; // Copy
        effect.effectID = a_ID;

        this->OriginalEffects.insert({ a_ID, effect });

        return effectData;
    }

    bool SpEffectParam::RestoreEffectById(int a_ID)
    {
        auto effectOffset = this->EffecIdtMap.find(a_ID);
        if (effectOffset == this->EffecIdtMap.end())
        {
            logger::println("Effect Id does not exist, at RestoreEffectById");
            return false;
        }

        auto originalEffect = this->OriginalEffects.find(a_ID);
        if (originalEffect == this->OriginalEffects.end())
        {
            logger::println("Effect was not modified yet!");
            return false;
        }

        EffectData* effectData = reinterpret_cast<EffectData*>(this->Base + effectOffset->second);
        *effectData = originalEffect->second.data;
        return true;

        return false;
    }

    void SpEffectParam::RestoreOriginalState()
    {
        for (auto originalEffect = this->OriginalEffects.begin(); originalEffect != this->OriginalEffects.end(); originalEffect++)
        {
            auto effectOffset = this->EffecIdtMap.find(originalEffect->second.effectID);
            if (effectOffset == this->EffecIdtMap.end())
            {
                logger::println("Effect Id does not exist, at RestoreOriginalState");
                continue;
            }

            EffectData* effectData = reinterpret_cast<EffectData*>(this->Base + effectOffset->second);
            *effectData = originalEffect->second.data;
        }
    }

    SpEffectParam::~SpEffectParam()
    {
        RestoreOriginalState();
    }

	uintptr_t SpEffectParam::GetBase() { return this->Base; }
	WORD SpEffectParam::GetSize() { return this->Count; }
	uintptr_t SpEffectParam::GetEffectIdAddress() { return this->EffectIdAddress; }
	uintptr_t SpEffectParam::GetEffectOffsetAddress() { return this->EffectOffsetAddress; }
}
