
#include "config.h"

namespace config
{
	int cycleSpeed = 100; // 100 is efficient enough to no skip vfx bugs, and fast enough to see
	bool InfusionViewerActive = true;
	bool PhantomColorActive = false;
	int PhantomSelfId = -1;
	int NetPlayer1Id = -1;
	int NetPlayer2Id = -1;
	int NetPlayer3Id = -1;
	int NetPlayer4Id = -1;
	int NetPlayer5Id = -1;

	bool ExtraVisualActive = false;
	int ExtraVFX = -1;
	int ExtraVFX1 = -1;
	int ExtraVFX2 = -1;
	int ExtraVFX3 = -1;
	int ExtraVFX4 = -1;
	int ExtraVFX5 = -1;
	int ExtraVFX6 = -1;
	int ExtraVFX7 = -1;

	namespace VFX
	{
		int g_FireEffectR = 62;
		int g_FireEffectL = 5100;

		//const int g_LightningEffectR = 8720; // alt 8720, 151
		int g_LightningEffectR = 151; // alt 8720, 151
		//const int g_LightningEffectL = 8721; // alt 8721, 5105;
		int g_LightningEffectL = 5105; // alt 8721, 5105;

		int g_SacradEffectR = 61;
		int g_SacradEffectL = 5140;

		int g_MagicEffectR = 64;
		int g_MagicEffectL = 5115;

		int g_ColdEffectR = 5092;
		int g_ColdEffectL = 5135;

		int g_PoisonEffectR = 152;
		int g_PoisonEffectL = 5110;

		int g_BloodEffectR = 5090;
		int g_BloodEffectL = 5125;
	}
}