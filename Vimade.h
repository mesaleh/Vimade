#pragma once
#include "helper.h"

BOOLEAN isSandbox();

BOOLEAN isVm_Drives();

BOOLEAN isVm_ComputerModel(IWbemServices *pSvc);

BOOLEAN isVM_NetworkAdapterName();