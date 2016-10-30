#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <Shlwapi.h>
#include "Vimade.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

BOOLEAN isSandbox()
{

}

BOOLEAN isVm_Drives()
{

	return FALSE;
}

BOOLEAN isVm_Bios(IWbemServices *pSvc)
{
	return FALSE;
}

BOOLEAN isVm_ComputerModel(IWbemServices *pSvc)
{
	IEnumWbemClassObject* pEnumerator = wmiExecQuery(pSvc, L"SELECT * FROM Win32_ComputerSystem");
	if (!pEnumerator)
	{
		printf("isVm_ComputerModel failed!\n");
		return FALSE;
	}

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	BOOLEAN Result = FALSE;
	VARIANT vtProp;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->lpVtbl->Next(pEnumerator, WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		hr = pclsObj->lpVtbl->Get(pclsObj, L"Model", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
		{
			printf("Computer Model: %ls\n", vtProp.bstrVal);
			if (StrStrI(vtProp.bstrVal, L"Virtual"))
			{
				Result = TRUE;
			}
		}		
	}
	VariantClear(&vtProp);
	if(pclsObj)	pclsObj->lpVtbl->Release(pclsObj);

	return Result;
}

BOOLEAN isVM_NetworkAdapterName()
{
	DWORD rv, Size = 15000;
	PIP_ADAPTER_ADDRESSES AdapterAddresses = NULL;
	BOOLEAN Result = TRUE;

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &Size);
	AdapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(Size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, AdapterAddresses, &Size);
	if (rv != ERROR_SUCCESS) {
		printf("GetAdaptersAddresses() failed...\n");
		free(AdapterAddresses);
		return FALSE;
	}

	printf("Network adapter name:\n");
	while (AdapterAddresses)
	{
		if (Size > 0)
		{
			printf("\t%ls\n", AdapterAddresses->Description);
			// detect if there is no real network adapter.
			if (!StrStrI(AdapterAddresses->Description, L"Virtual") && !StrStrI(AdapterAddresses->Description, L"loopback"))
			{
				Result = FALSE;
			}
		}
		AdapterAddresses = AdapterAddresses->Next;
	}

	free(AdapterAddresses);
	return Result;
}
