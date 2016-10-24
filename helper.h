#pragma once
#include <Windows.h>
#include <Wbemidl.h>

IWbemServices* wmiInit();

IEnumWbemClassObject* wmiExecQuery(IWbemServices *pSvc, wchar_t* query);

void wmiClean(IWbemServices *pSvc);

void dropFile(char* name);