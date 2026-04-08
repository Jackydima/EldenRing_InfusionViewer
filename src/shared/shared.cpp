#include "shared.h"

namespace bases
{
    namespace characterEquipmentOffset
    {
        const std::vector<intptr_t> playerInstance = { 0x10EF8, 0, 0 };

        const std::vector<intptr_t> primRightWep = { 0x8, 0x39C };
        const std::vector<intptr_t> primLeftWep = { 0x8, 0x398 };

        const std::vector<intptr_t> secRightWep = { 0x8, 0x3A4 };
        const std::vector<intptr_t> secLeftWep = { 0x8, 0x3A0 };

        const std::vector<intptr_t> tertRightWep = { 0x8, 0x3AC };
        const std::vector<intptr_t> tertLeftWep = { 0x8, 0x3A8 };

        const std::vector<intptr_t> currentLeftWep = { 0x8, 0x328 };
        const std::vector<intptr_t> currentRightWep = { 0x8, 0x32C };
        const std::vector<intptr_t> currentArmStyle = { 0x8, 0x324 };
    }
    // Foreward declarations
    static bool initCodeSegments();
    static uintptr_t getRIPAddress(uintptr_t base, size_t a_lInstrLen = 7ull, size_t a_lDataPtrOffset = 3ull);
    // Foreward declarations END

    static uintptr_t StartAddress = 0;
    static size_t SizeOfVirtualMem = 0;

	uintptr_t GameDataMan = NULL;
	uintptr_t WorldChrMan = NULL;
	uintptr_t SoloParamRepository = NULL;
    SpEffectParam SpEffectParamInst;

    AddEffect_t g_AddEffect = nullptr;
    RemoveEffect_t g_RemoveEffect = nullptr;

	bool initialize()
	{
        if (!initCodeSegments())
        {
            logger::println("Could not initilialize pe header segments");
            return false;
        }

        uintptr_t result;

        // TGA Table AOB string
        result = tools::memory::searchUniqueAOB(StartAddress, SizeOfVirtualMem, "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 05 48 8B 40 58 C3 C3", '?');
        if (result == 0)
            return false;

        GameDataMan = getRIPAddress(result);
        logger::println("GameDataMan: %p", reinterpret_cast<LPVOID>(GameDataMan));

        result = tools::memory::searchUniqueAOB(StartAddress, SizeOfVirtualMem, "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88", '?');
        if (result == 0)
            return false;

        WorldChrMan = getRIPAddress(result);
        logger::println("WorldChrMan: %p", reinterpret_cast<LPVOID>(WorldChrMan));

        // TGA Table AOB string
        result = tools::memory::searchUniqueAOB(StartAddress, SizeOfVirtualMem, "48 89 5C 24 48 8B FA 48 8B D9 C7 44 24 20 00 00 00 00 48", '?');
        if (result == 0)
            return false;

        result += 18; // aob pattern offset of instruction!
        SoloParamRepository = getRIPAddress(result);
        logger::println("SoloParamRepository: %p", reinterpret_cast<LPVOID>(SoloParamRepository));

        SpEffectParamInst.init(SoloParamRepository);

        g_AddEffect = reinterpret_cast<AddEffect_t>(tools::memory::searchUniqueAOB(StartAddress, SizeOfVirtualMem, "0f 28 0d ?? ?? ?? ?? ?? 8d ?? ?? 0f 29 ?? ?? ?? 0f b6 d8", '?') - 0x1D); // correction!
        logger::println("Address of g_AddEffect: %p", reinterpret_cast<LPVOID>(g_AddEffect));
        if (g_AddEffect == 0)
            return false;

        g_RemoveEffect = reinterpret_cast<RemoveEffect_t>(tools::memory::searchUniqueAOB(StartAddress, SizeOfVirtualMem, "48 83 EC 28 8B C2 48 8B 51 08 48 85 D2 ?? ?? 90", '?'));
        logger::println("Address of g_RemoveEffect: %p", reinterpret_cast<LPVOID>(g_RemoveEffect));
        if (g_RemoveEffect == 0)
            return false;

        return true;
	}

    static uintptr_t getRIPAddress(uintptr_t base, size_t a_lInstrLen, size_t a_lDataPtrOffset)
    {
        DWORD offSet = *reinterpret_cast<DWORD*>(base + a_lDataPtrOffset); // skip first 3 bytes for ">mov rax<,[data]"
        return base + a_lInstrLen + offSet;
    }

    static bool initCodeSegments()
    {
        HMODULE eldenringModule = GetModuleHandleW(NULL);
        if (eldenringModule == NULL)
        {
            logger::println("Could not find Elden Ring Module");
            return false;
        }

        IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)eldenringModule;
        IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);

        if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE)
        {
            logger::println("Signature of Image_NT_Headers does not match!");
            return false;
        }

        IMAGE_SECTION_HEADER* pImageSectionHeader = IMAGE_FIRST_SECTION(pNTHeaders);
        IMAGE_FILE_HEADER pImageFileHeader = pNTHeaders->FileHeader;

        size_t index = 0;
        while (index < pImageFileHeader.NumberOfSections)
        {
            logger::println("Name of section: %s", pImageSectionHeader->Name);
            logger::println("Virtual Address: %d", pImageSectionHeader->VirtualAddress);
            logger::println("Size: %d", pImageSectionHeader->Misc.VirtualSize);

            if (0 == strcmp(".text", (const char*)pImageSectionHeader->Name))
            {
                StartAddress = pImageSectionHeader->VirtualAddress + pNTHeaders->OptionalHeader.ImageBase;
                SizeOfVirtualMem = pImageSectionHeader->Misc.VirtualSize;
            }

            // Extension!
            if (0 == strcmp(".interpr", (const char*)pImageSectionHeader->Name))
            {
                SizeOfVirtualMem += pImageSectionHeader->Misc.VirtualSize;
                break;
            }
            pImageSectionHeader++;
            index++;
        }

        return true;
    }
}