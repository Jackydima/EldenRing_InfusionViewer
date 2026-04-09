#include "spEffectParam.h"

namespace bases
{
    SpEffectParam::SpEffectParam(): ParamBase() {}

    bool SpEffectParam::init(uintptr_t SoloParamRepository)
    {
        this->m_Base = memory::readPointerSafe<uintptr_t*>(SoloParamRepository, { 0x4C0, 0x80, 0x80 });
        logger::println("SpEffectParam: %p", reinterpret_cast<LPVOID>(this->m_Base));
        if (!memory::isReadable(reinterpret_cast<uintptr_t>(m_Base)))
            return false;

        this->m_Count = *memory::readOffSet<WORD*>(*this->m_Base, 0x0A);
        this->m_IdAddressStart = *this->m_Base + 0x40;
        this->m_OffsetAddressStart = *this->m_Base + 0x48;

        int32_t currentID;
        int32_t currentOffset;

        for (WORD i = 0; i < this->m_Count; i++)
        {
            currentID = *memory::readOffSet<int32_t*>(this->m_IdAddressStart, i * EffectIdAddressJumpOffset);
            currentOffset = *memory::readOffSet<int32_t*>(this->m_OffsetAddressStart, i * EffectIdAddressJumpOffset);
            this->EffecIdMap.insert({currentID, currentOffset});
        }
        return true;
    }

    EffectData* SpEffectParam::GetEffectById(int a_ID)
    {
        auto effectOffset = this->EffecIdMap.find(a_ID);
        if (effectOffset == this->EffecIdMap.end())
        {
            logger::println("Id not found in GetEffectById");
            return nullptr;
        }

        return reinterpret_cast<EffectData*>(*this->m_Base + effectOffset->second);
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
        auto effectOffset = this->EffecIdMap.find(a_ID);
        if (effectOffset == this->EffecIdMap.end())
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

        EffectData* effectData = reinterpret_cast<EffectData*>(*this->m_Base + effectOffset->second);
        *effectData = originalEffect->second.data;
        return true;

        return false;
    }

    void SpEffectParam::RestoreOriginalState()
    {
        for (auto originalEffect = this->OriginalEffects.begin(); originalEffect != this->OriginalEffects.end(); originalEffect++)
        {
            auto effectOffset = this->EffecIdMap.find(originalEffect->second.effectID);
            if (effectOffset == this->EffecIdMap.end())
            {
                logger::println("Effect Id does not exist, at RestoreOriginalState");
                continue;
            }

            EffectData* effectData = reinterpret_cast<EffectData*>(*this->m_Base + effectOffset->second);
            *effectData = originalEffect->second.data;
        }
    }

    SpEffectParam::~SpEffectParam()
    {
        RestoreOriginalState();
    }
}
