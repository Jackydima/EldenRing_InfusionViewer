#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <thread>

#include "debug_print.h"


namespace tools
{
	namespace memory
	{
		struct PatternItem
		{
			const BYTE m_Value;
			const BYTE m_Mask;

			PatternItem(): m_Value(0x00), m_Mask(0xFF) {}
			PatternItem(BYTE a_Value, BYTE a_Mask): m_Value(a_Value), m_Mask(a_Mask) {}
		};

		uintptr_t searchUniqueAOB(uintptr_t p_uStartAddress, size_t p_uSize, const std::string& p_strPattern, const char p_cWildCard);

		inline BYTE hexCharToNibble(char c)
		{
			if (c >= '0' && c <= '9') return c - '0';
			if (c >= 'A' && c <= 'F') return c - 'A' + 10;
			if (c >= 'a' && c <= 'f') return c - 'a' + 10;

			return 0xFF; // invalid
		}

		inline bool hexPairToByte(char high, char low, BYTE& outByte)
		{
			BYTE hi = hexCharToNibble(high);
			BYTE lo = hexCharToNibble(low);

			if (hi == 0xFF || lo == 0xFF)
				return false;

			outByte = (hi << 4) | lo;
			return true;
		}
	}
}