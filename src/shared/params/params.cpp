#include "params.h"

ParamBase::ParamBase() : m_Base(0), m_Count(0), m_IdAddressStart(0), m_OffsetAddressStart(0) {}

ParamBase::~ParamBase()
{
	RestoreOriginalState();
}

bool ParamBase::init(uintptr_t SoloParamRepository) { return true; }
void ParamBase::RestoreOriginalState() {}

uintptr_t* ParamBase::GetBase() { return this->m_Base; }
WORD ParamBase::GetSize() { return this->m_Count; }
uintptr_t ParamBase::GetIdAddressStart() { return this->m_IdAddressStart; }
uintptr_t ParamBase::GetOffsetAddressStart() { return this->m_OffsetAddressStart; }