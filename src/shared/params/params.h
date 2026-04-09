#pragma once

#include <Windows.h>

class ParamBase
{
protected:
	uintptr_t* m_Base;
	WORD m_Count;
	uintptr_t m_IdAddressStart;
	uintptr_t m_OffsetAddressStart;

public:
	ParamBase();
	~ParamBase();

public:
	virtual bool init(uintptr_t SoloParamRepository);
	virtual void RestoreOriginalState();
	uintptr_t* GetBase();
	WORD GetSize();
	uintptr_t GetIdAddressStart();
	uintptr_t GetOffsetAddressStart();
};