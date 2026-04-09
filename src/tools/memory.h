#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <thread>

#include "debug_print.h"

namespace memory
{
	struct PatternItem
	{
		const BYTE m_Value;
		const BYTE m_Mask;

		PatternItem(): m_Value(0x00), m_Mask(0xFF) {}
		PatternItem(BYTE a_Value, BYTE a_Mask): m_Value(a_Value), m_Mask(a_Mask) {}
	};

	inline bool isReadable(uintptr_t address)
	{
		if (!address)
			return false;

		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery(reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT)
			return false;

		if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		return true;
	}

	template <typename T>
	requires std::is_pointer_v<T>
	inline T readPointerSafe(uintptr_t a_lBase, const std::vector<intptr_t>& a_lOffsets)
	{
		if (!a_lBase)
			return reinterpret_cast<T>(nullptr);

		if (a_lOffsets.empty())
			return reinterpret_cast<T>(a_lBase);

		uintptr_t current = a_lBase;
		for (size_t offset : a_lOffsets)
		{
			if (!isReadable(current))
				return reinterpret_cast<T>(nullptr);

			current = *reinterpret_cast<uintptr_t*>(current);
			if (!current)
				return reinterpret_cast<T>(nullptr);

			current += offset;
		}

		return reinterpret_cast<T>(current);
	}

	template <typename T>
	inline T readOffSet(uintptr_t a_lBase, int64_t a_lOffset)
	{
		if (a_lBase == 0)
			return reinterpret_cast<T>(nullptr);

		return reinterpret_cast<T>(a_lBase + a_lOffset);
	}

	template <typename T>
	inline bool writePointer(uintptr_t a_lBase, T a_Value)
	{
		if (a_lBase == 0)
			return false;

		*reinterpret_cast<T*>(a_lBase) = a_Value;
		return true;
	}

	uintptr_t searchUniqueAOB(uintptr_t p_uStartAddress, size_t p_uSize, const std::string& p_strPattern, const char a_cWildCard = '?');

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