#pragma once

namespace config
{
	extern int cycleSpeed;
	extern bool InfusionViewerActive;
	extern bool PhantomColorActive;
	extern int PhantomSelfId;
	extern int NetPlayer1Id;
	extern int NetPlayer2Id;
	extern int NetPlayer3Id;
	extern int NetPlayer4Id;
	extern int NetPlayer5Id;

	extern bool ExtraVisualActive;
	extern int ExtraVFX;
	extern int ExtraVFX1;
	extern int ExtraVFX2;
	extern int ExtraVFX3;
	extern int ExtraVFX4;
	extern int ExtraVFX5;
	extern int ExtraVFX6;
	extern int ExtraVFX7;

	namespace VFX
	{
		extern int g_FireEffectR;
		extern int g_FireEffectL;

		extern int g_LightningEffectR; // alt 8720, 151
		extern int g_LightningEffectL; // alt 8721, 5105;

		extern int g_SacradEffectR;
		extern int g_SacradEffectL;

		extern int g_MagicEffectR;
		extern int g_MagicEffectL;

		extern int g_ColdEffectR;
		extern int g_ColdEffectL;

		extern int g_PoisonEffectR;
		extern int g_PoisonEffectL;

		extern int g_BloodEffectR;
		extern int g_BloodEffectL;
	}
}