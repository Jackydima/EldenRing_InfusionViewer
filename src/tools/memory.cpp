
#include "memory.h"

constexpr size_t BUFFER_SIZE = 256;

namespace memory
{
	// Foreward Declarations
	static bool serializePattern(std::vector<PatternItem>& p_retPattern, std::string p_strPattern, const char p_cWildCard);
	static size_t comparePattern(BYTE* a_pUStartAddressPtr, const std::vector<PatternItem>& a_rVecRetPattern, int a_rIBadBytes[BUFFER_SIZE]);
	static void prepareBadBytes(const std::vector<PatternItem>& p_retPattern, int a_rIBadBytes[BUFFER_SIZE]);
	// Foreward Declarations END

	/*uintptr_t searchUniqueAOBMultiThread(uintptr_t a_uStartAddress, size_t a_uSize, const std::string& a_rStrPattern, const char a_cWildCard = '?', int a_IThreadWorkers = 4)
	{
		int realWorkerAmount = a_IThreadWorkers;
		size_t uNewSize = a_uSize / 4;
	}*/

	uintptr_t searchUniqueAOB(uintptr_t a_uStartAddress, size_t a_uSize, const std::string& a_rStrPattern, const char a_cWildCard)
	{
		uintptr_t uReturnVal = 0;

		if (a_rStrPattern.size() < 2)
		{
			logger::println("pattern size must atleast have 1 whole byte in it!");
			return uReturnVal;
		}

		if (a_uSize < a_rStrPattern.size())
		{
			logger::println("size of checked address space is even smaller than the pattern...");
			return uReturnVal;
		}

		std::vector<PatternItem> bytePattern;
		if (!serializePattern(bytePattern, a_rStrPattern, a_cWildCard))
		{
			return uReturnVal;
		}


		size_t bytePatternSize = bytePattern.size();
		uintptr_t uCurrentAddress = a_uStartAddress;
		uintptr_t uEndAddress = a_uStartAddress + a_uSize;

		int iBadBytes[BUFFER_SIZE];
		prepareBadBytes(bytePattern, iBadBytes);
			
		size_t uResult;
		while ((uCurrentAddress + bytePatternSize) <= uEndAddress)
		{
			uResult = comparePattern(reinterpret_cast<BYTE*>(uCurrentAddress), bytePattern, iBadBytes);
			if (uResult == 0)
			{
				logger::println("Found Pattern!");
				if (uReturnVal != 0)
				{
					logger::println("Already found Pattern at %x, but found another one at %x", uReturnVal, uCurrentAddress);
					return 0;
				}
				uReturnVal = uCurrentAddress;

				uCurrentAddress += 1;
				continue;
			}

			uCurrentAddress += (uResult > 0) ? uResult : 1;
		}

		return uReturnVal;
	}

	static size_t comparePattern(BYTE* a_pUStartAddressPtr, const std::vector<PatternItem>& a_rVecRetPattern, int a_rIBadBytes[BUFFER_SIZE])
	{
		BYTE u8Mask;
		BYTE u8Value;
		BYTE u8TargetValue;
		int iIndex;
		for (int i = static_cast<int>(a_rVecRetPattern.size()) - 1; i >= 0; i--)
		{
			u8Mask = a_rVecRetPattern[i].m_Mask;
			u8Value = a_rVecRetPattern[i].m_Value;
			u8TargetValue = a_pUStartAddressPtr[i];

			if ((u8TargetValue & u8Mask) == (u8Value & u8Mask))
				continue;

			iIndex = a_rIBadBytes[u8TargetValue];

			if (iIndex == -1) // Not included
			{
				// Shift whole pattern size to the right!
				return a_rVecRetPattern.size();
			}

			// shift to the right to match bad byte!
			int iShift = i - iIndex;
			if (iShift <= 0) // Should not happen
				iShift = 1;

			return static_cast<size_t>(iShift);
		}
		return 0;
	}

	static void prepareBadBytes(const std::vector<PatternItem>& p_retPattern, int a_rIBadBytes[BUFFER_SIZE])
	{
		size_t patternSize = p_retPattern.size();

		int lastWildcardIdx = -1;
		for (int i = static_cast<int>(patternSize) - 1; i >= 0; i--) {
			if (p_retPattern[i].m_Mask != 0xFF) {
				lastWildcardIdx = i;
				break;
			}
		}

		for (int i = 0; i < BUFFER_SIZE; i++) {
			a_rIBadBytes[i] = lastWildcardIdx;
		}

		// Fill known bytes that appear AFTER the last wildcard
		for (int i = lastWildcardIdx + 1; i < static_cast<int>(patternSize) - 1; i++) {
			a_rIBadBytes[p_retPattern[i].m_Value] = i;
		}
	}

	static bool serializePattern(std::vector<PatternItem>& p_retPattern, std::string p_strPattern, const char p_cWildCard)
	{			
		// Remove all spaces
		p_strPattern.erase(std::remove_if(p_strPattern.begin(), p_strPattern.end(), [](unsigned char c) { return std::isspace(c); }), p_strPattern.end());

		// Only Allow even string size for byte representations!
		if (p_strPattern.size() % 2 != 0)
		{
			logger::println("odd hexadezimal number string given!");
			return false;
		}

		p_retPattern.clear();
		p_retPattern.reserve(p_strPattern.size() / 2); // Optimization

		for (auto currentPattern = p_strPattern.begin(); currentPattern != p_strPattern.end(); currentPattern += 2)
		{
			auto currentPatternSecond = currentPattern + 1;

			bool firstIsWildcard = (*currentPattern == p_cWildCard);
			bool secondIsWildcard = (*currentPatternSecond == p_cWildCard);

			if (firstIsWildcard && secondIsWildcard)
			{
				p_retPattern.push_back(PatternItem(0x00, 0x00));
				continue;
			}

			BYTE lo = hexCharToNibble(*currentPatternSecond);
			if (firstIsWildcard)
			{
				if (lo == 0xFF)
				{
					logger::println("Invalid hex byte code at char: %c", *currentPatternSecond);
					return false;
				}

				p_retPattern.push_back(PatternItem(lo, 0x0F));
				continue;
			}

			BYTE hi = hexCharToNibble(*currentPattern);
			if (secondIsWildcard)
			{
				if (hi == 0xFF)
				{
					logger::println("Invalid hex byte code at char: %c", *currentPattern);
					return false;
				}

				p_retPattern.push_back(PatternItem(hi << 4, 0xF0));
				continue;
			}

			if (hi == 0xFF || lo == 0xFF)
			{
				logger::println("Invalid hex byte code at char: %c, char: %c", *currentPattern, *currentPatternSecond);
				return false;
			}

			p_retPattern.push_back(PatternItem((hi << 4) | lo, 0xFF));
		}

		return true;
	}
}
